#include "interpreter/interpreter.hpp"
#include <iostream>

namespace interpreter {

void Interpreter::interpret(const std::vector<std::unique_ptr<ast::Stmt>>& statements) {
    try {
        for (const auto& stmt : statements) {
            stmt->accept(*this);
        }
        
        Function* mainFunc = currentEnv->getFunction("main");
        if (mainFunc) {
            callFunction(mainFunc, {});
        }
    }
    catch (const std::runtime_error& err) {
        std::cerr << "Runtime error: " << err.what() << std::endl;
    }
}

Value Interpreter::getResult() const {
    if (evalStack.empty())
        return Value();
    return evalStack.back();
}

void Interpreter::evaluate(ast::Expr& expr) {
    expr.accept(*this);
}

Value Interpreter::popValue() {
    if (evalStack.empty()) {
        throw std::runtime_error("No value on stack");
    }

    Value val = std::move(evalStack.back());
    evalStack.pop_back();

    return val;
}

void Interpreter::applyBinaryOp(ast::Op op) {
    Value right = popValue();
    Value left = popValue();
    
    if (!left.isNumber() || !right.isNumber()) {
        throw std::runtime_error("Binary operations require numbers");
    }
    
    double result;
    switch (op) {
        case ast::Op::Plus:
            result = left.asNumber() + right.asNumber();
            break;
        case ast::Op::Minus:
            result = left.asNumber() - right.asNumber();
            break;
        case ast::Op::Multiply:
            result = left.asNumber() * right.asNumber();
            break;
        case ast::Op::Divide:
            if (right.asNumber() == 0) {
                throw std::runtime_error("Division by zero");
            }
            result = left.asNumber() / right.asNumber();
            break;
        default:
            throw std::runtime_error("Unknown operator");
    }
    
    evalStack.push_back(Value(result));
}

void Interpreter::applyComparisonOp(ast::ComparisonOp op) {
    Value right = popValue();
    Value left = popValue();
    
    bool result = false;
    
    if (left.isNumber() && right.isNumber()) {
        double l = left.asNumber();
        double r = right.asNumber();
        
        switch (op) {
            case ast::ComparisonOp::Equal:
                result = l == r;
                break;
            case ast::ComparisonOp::NotEqual:
                result = l != r;
                break;
            case ast::ComparisonOp::Less:
                result = l < r;
                break;
            case ast::ComparisonOp::LessEqual:
                result = l <= r;
                break;
            case ast::ComparisonOp::Greater:
                result = l > r;
                break;
            case ast::ComparisonOp::GreaterEqual:
                result = l >= r;
                break;
        }
    }
    else if (left.isString() && right.isString()) {
        std::string l = left.asString();
        std::string r = right.asString();
        
        switch (op) {
            case ast::ComparisonOp::Equal:
                result = l == r;
                break;
            case ast::ComparisonOp::NotEqual:
                result = l != r;
                break;
            default:
                throw std::runtime_error("String comparison only supports == and !=");
        }
    }
    else {
        throw std::runtime_error("Cannot compare different types");
    }
    
    evalStack.push_back(Value(result));
}

void Interpreter::visit(ast::BinaryExpr& expr) {
    evaluate(*expr.left);
    evaluate(*expr.right);
    applyBinaryOp(expr.op);
}

void Interpreter::visit(ast::ComparisonExpr& expr) {
    evaluate(*expr.left);
    evaluate(*expr.right);
    applyComparisonOp(expr.op);
}

void Interpreter::visit(ast::LiteralNumber& expr) {
    evalStack.push_back(Value(expr.value));
}

void Interpreter::visit(ast::LiteralString& expr) {
    evalStack.push_back(Value(expr.value));
}

void Interpreter::visit(ast::Identifier& expr) {
    try {
        Value val = currentEnv->getVariable(expr.name);
        evalStack.push_back(val);
        return;
    }
    catch (const std::runtime_error&) {}
    
    Function* func = currentEnv->getFunction(expr.name);
    if (func) {
        callFunction(func, {});
        return;
    }
    
    throw std::runtime_error("Undefined variable or function: " + expr.name);
}

void Interpreter::callFunction(Function* func, const std::vector<Value>& args) {
    Environment* funcEnv = currentEnv->createChild();
    Environment* previousEnv = currentEnv;
    currentEnv = funcEnv;
    
    for (size_t i = 0; i < func->params.size() && i < args.size(); i++) {
        funcEnv->defineVariable(func->params[i].second, args[i]);
    }
    
    hasReturned = false;
    func->body->accept(*this);
    
    delete currentEnv;
    currentEnv = previousEnv;
    
    if (hasReturned) {
        evalStack.push_back(returnValue);
        hasReturned = false;
    }
}

void Interpreter::visit(ast::VariableDeclarationStmt& stmt) {
    Value initialValue;
    
    if (stmt.initializer) {
        evaluate(*stmt.initializer);
        initialValue = popValue();
    }
    else {
        if (stmt.type == "int" || stmt.type == "double" || stmt.type == "float" || stmt.type == "number") {
            initialValue = Value(0.0);
        }
        else if (stmt.type == "string") {
            initialValue = Value(std::string(""));
        }
        else if (stmt.type == "bool" || stmt.type == "boolean") {
            initialValue = Value(false);
        }
        else {
            initialValue = Value();
        }
    }
    
    currentEnv->defineVariable(stmt.name, initialValue);
}

void Interpreter::visit(ast::ExpressionStmt& stmt) {
    evaluate(*stmt.expr);
    popValue();
}

void Interpreter::visit(ast::AssignStmt& stmt) {
    evaluate(*stmt.value);
    Value value = popValue();
    
    currentEnv->setVariable(stmt.name, value);
}

void Interpreter::visit(ast::BlockStmt& stmt) {
    Environment* previousEnv = currentEnv;
    currentEnv = currentEnv->createChild();
    
    for (const auto& statement : stmt.statements) {
        statement->accept(*this);
        if (hasReturned) {
            break;
        }
    }
    
    delete currentEnv;
    currentEnv = previousEnv;
}

void Interpreter::visit(ast::IfStmt& stmt) {
    evaluate(*stmt.condition);
    Value condition = popValue();
    
    bool condValue = false;
    if (condition.isNumber()) {
        condValue = condition.asNumber() != 0;
    }
    else if (condition.isBoolean()) {
        condValue = condition.asBoolean();
    }
    else {
        throw std::runtime_error("Condition must be boolean or number");
    }
    
    if (condValue) {
        stmt.thenBranch->accept(*this);
    }
    else if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
}

void Interpreter::visit(ast::ForStmt& stmt) {
    Environment* loopEnv = currentEnv->createChild();
    Environment* previousEnv = currentEnv;
    currentEnv = loopEnv;
    
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
    }
    
    while (true) {
        if (stmt.condition) {
            evaluate(*stmt.condition);
            Value condition = popValue();
            bool condValue = condition.isNumber() ? condition.asNumber() != 0 : condition.asBoolean();
            if (!condValue)
                break;
        }
        
        stmt.body->accept(*this);
        if (hasReturned)
            break;
        
        if (stmt.increment) {
            stmt.increment->accept(*this);
        }
    }
    
    delete loopEnv;
    currentEnv = previousEnv;
}

void Interpreter::visit(ast::FunctionStmt& stmt) {
    Function func;
    func.name = stmt.name;
    func.params = stmt.params;
    func.body = std::move(const_cast<std::unique_ptr<ast::Stmt>&>(stmt.body));
    func.returnType = stmt.returnType;
    
    currentEnv->defineFunction(stmt.name, std::move(func));
}

void Interpreter::visit(ast::FunctionCall& call) {
    std::vector<Value> args;
    for (auto& arg : call.args) {
        evaluate(*arg);
        args.push_back(popValue());
    }
    
    Function* func = currentEnv->getFunction(call.name);
    if (!func) {
        throw std::runtime_error("Undefined function: " + call.name);
    }
    
    callFunction(func, args);
}

void Interpreter::visit(ast::ReturnStmt& stmt) {
    if (stmt.value) {
        evaluate(*stmt.value);
        returnValue = popValue();
    }
    else {
        returnValue = Value();
    }

    hasReturned = true;
}

void Interpreter::visit(ast::EchoStmt& stmt) {
    evaluate(*stmt.expr);
    Value value = popValue();

    std::cout << value.toString() << std::endl;
}

} // namespace interpreter
