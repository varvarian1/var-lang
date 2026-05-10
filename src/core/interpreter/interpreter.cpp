#include "interpreter/interpreter.hpp"
#include <iostream>
#include <memory>

namespace interpreter {

void Interpreter::interpret(const std::vector<ast::Statement>& statements) {
    try {
        for (const auto& stmt : statements) {
            stmt->accept(*this);
        }
        
        std::optional<Function::Ref> mainFunc = currentEnv->getFunction("main");
        if (mainFunc) {
            callFunction(*mainFunc, {});
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

void Interpreter::applyBinaryOperator(ast::BinaryOperator op) {
    Value right = popValue();
    Value left = popValue();
    
    Value result;

    if (op == ast::BinaryOperator::Assign) {
        // Preserve declared numeric kind (int vs float) on assignment.
        // We still store everything as double, but int values are truncated.
        Value target = currentEnv->getVariable(left.asString());
        if (target.isNumber() && right.isNumber()) {
            if (target.numberKind == Value::NumberKind::Int) {
                right = Value(static_cast<double>(static_cast<long long>(right.asNumber())), Value::NumberKind::Int);
            } else {
                right.numberKind = Value::NumberKind::Float;
            }
        }
        currentEnv->setVariable(left.asString(), right);
        result = right;
    } else if (left.isNumber() && right.isNumber()) {
        double l = left.asNumber();
        double r = right.asNumber();
        bool ints = (left.numberKind == Value::NumberKind::Int) && (right.numberKind == Value::NumberKind::Int);
        
        switch (op) {
            case ast::BinaryOperator::Add:
                result = Value(left.asNumber() + right.asNumber(), ints ? Value::NumberKind::Int : Value::NumberKind::Float);
                break;
            case ast::BinaryOperator::Subtract:
                result = Value(left.asNumber() - right.asNumber(), ints ? Value::NumberKind::Int : Value::NumberKind::Float);
                break;
            case ast::BinaryOperator::Multiply:
                result = Value(left.asNumber() * right.asNumber(), ints ? Value::NumberKind::Int : Value::NumberKind::Float);
                break;
            case ast::BinaryOperator::Divide:
                if (right.asNumber() == 0) {
                    throw std::runtime_error("Division by zero");
                }
                if (ints) {
                    result = Value(static_cast<double>(static_cast<long long>(l) / static_cast<long long>(r)), Value::NumberKind::Int);
                } else {
                    result = Value(left.asNumber() / right.asNumber(), Value::NumberKind::Float);
                }
                break;
            case ast::BinaryOperator::Equal:
                result = l == r;
                break;
            case ast::BinaryOperator::NotEqual:
                result = l != r;
                break;
            case ast::BinaryOperator::Less:
                result = l < r;
                break;
            case ast::BinaryOperator::LessEqual:
                result = l <= r;
                break;
            case ast::BinaryOperator::Greater:
                result = l > r;
                break;
            case ast::BinaryOperator::GreaterEqual:
                result = l >= r;
                break;
            default:
                throw std::runtime_error("Unsupported operator for numbers");
        }
    } else if (left.isString() && right.isString()) {
        std::string l = left.asString();
        std::string r = right.asString();
        
        switch (op) {
            case ast::BinaryOperator::Equal:
                result = l == r;
                break;
            case ast::BinaryOperator::NotEqual:
                result = l != r;
                break;
            default:
                throw std::runtime_error("String comparison only supports == and !=");
        }
    } else {
        throw std::runtime_error("Cannot apply operator to different types");
    }
    
    evalStack.push_back(Value(result));
}

void Interpreter::applyUnaryOperator(ast::UnaryOperator op, ast::Expr& operand) {
    // for prefix --x / ++x and postfix x++ / x-- we need an lvalue, not a value
    // for UnaryMinus / UnaryPlus a value is sufficient
    
    if (op == ast::UnaryOperator::UnaryMinus) {
        Value v = popValue();
        evalStack.push_back(Value(-v.asNumber(), v.numberKind));
        return;
    }
    if (op == ast::UnaryOperator::UnaryPlus) {
        return;   // unchanged
    }
    
    // ++/-- require operand to be an Identifier (lvalue)
    auto* ident = dynamic_cast<ast::Identifier*>(&operand);
    if (!ident) throw std::runtime_error("++/-- requires variable");
    
    popValue();   // discard what evaluate put on the stack (the old value)
    Value old = currentEnv->getVariable(ident->name);
    Value newVal = Value(old.asNumber() + 
        (op == ast::UnaryOperator::PreIncrement || op == ast::UnaryOperator::PostIncrement ? 1 : -1), old.numberKind);
    currentEnv->setVariable(ident->name, newVal);
    
    bool is_prefix = (op == ast::UnaryOperator::PreIncrement || op == ast::UnaryOperator::PreDecrement);
    evalStack.push_back(is_prefix ? newVal : old);   // pre returns new value, post returns old
}

void Interpreter::visit(ast::BinaryExpr& expr) {
    if (expr.op == ast::BinaryOperator::Comma) {
        evaluate(*expr.left);
        popValue();
        evaluate(*expr.right);
        return;
    }
    
    evaluate(*expr.left);
    evaluate(*expr.right);
    applyBinaryOperator(expr.op);
}

void Interpreter::visit(ast::UnaryExpr& expr) {
    evaluate(*expr.left);
    applyUnaryOperator(expr.op, *expr.left);
}

void Interpreter::visit(ast::LiteralNumber& expr) {
    evalStack.push_back(Value(
        expr.value,
        expr.type == ast::LiteralNumber::Type::INT ? Value::NumberKind::Int : Value::NumberKind::Float));
}

void Interpreter::visit(ast::LiteralString& expr) {
    evalStack.push_back(Value(expr.value));
}

void Interpreter::visit(ast::Identifier& expr) {
    Value val = currentEnv->getVariable(expr.name);
    evalStack.push_back(val);
}

void Interpreter::callFunction(Function& func, const std::vector<Value>& args) {
    auto funcEnv = currentEnv->createChild();
    auto previousEnv = currentEnv;
    currentEnv = funcEnv;

    if (func.params.size() != args.size()) {
        currentEnv = previousEnv;
        throw std::runtime_error("Function '" + func.name + "' expected " +
            std::to_string(func.params.size()) + " args, got " + 
            std::to_string(args.size()));
    }

    for (size_t i = 0; i < func.params.size(); ++i) {
        currentEnv->defineVariable(func.params[i]->name, args[i]);
    }

    hasReturned = false;
    func.body->accept(*this);

    currentEnv = previousEnv;

    if (hasReturned) {
        evalStack.push_back(returnValue);
        hasReturned = false;
    } else {
        evalStack.push_back(Value());
    }
}

void Interpreter::visit(ast::VarDeclExpr& expr) {
    Value initialValue;
    
    if (expr.type == "int" || expr.type == "double" || expr.type == "float" || expr.type == "number") {
        initialValue = Value(0.0, expr.type == "int" ? Value::NumberKind::Int : Value::NumberKind::Float);
    } else if (expr.type == "string") {
        initialValue = Value(std::string(""));
    } else if (expr.type == "bool" || expr.type == "boolean") {
        initialValue = Value(false);
    } else {
        initialValue = Value();
    }
    
    currentEnv->defineVariable(expr.name, initialValue);

    evalStack.push_back(Value(expr.name));
}

void Interpreter::visit(ast::ExpressionStmt& stmt) {
    evaluate(*stmt.expr);
    popValue();
}

void Interpreter::visit(ast::ReturnStmt& stmt) {
    evaluate(*stmt.value);
    returnValue = popValue();
    
    hasReturned = true;
}

void Interpreter::visit(ast::BlockStmt& stmt) {
    std::shared_ptr<Environment> previousEnv = currentEnv;
    currentEnv = currentEnv->createChild();
    
    for (const auto& statement : stmt.statements) {
        statement->accept(*this);
        if (hasReturned) {
            break;
        }
    }
    
    currentEnv = previousEnv;
}

void Interpreter::visit(ast::IfStmt& stmt) {
    evaluate(*stmt.condition);
    Value condition = popValue();
    
    bool condValue = false;
    if (condition.isNumber()) {
        condValue = condition.asNumber() != 0;
    } else if (condition.isBoolean()) {
        condValue = condition.asBoolean();
    } else {
        throw std::runtime_error("Condition must be boolean or number");
    }
    
    if (condValue) {
        stmt.thenBranch->accept(*this);
    } else if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
}

void Interpreter::visit(ast::ForStmt& stmt) {
    std::shared_ptr<Environment> loopEnv = currentEnv->createChild();
    std::shared_ptr<Environment> previousEnv = currentEnv;
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
    
    currentEnv = previousEnv;
}

void Interpreter::visit(ast::FunctionDecl& stmt) {
    Function func;
    func.name = stmt.name;
    func.body = stmt.body.get();
    func.returnType = stmt.returnType;
    
    if (stmt.params)
        flatten_params(stmt.params.get(), func.params);
    
    currentEnv->defineFunction(stmt.name, std::move(func));
}

void Interpreter::visit(ast::FunctionCall& call) {
    std::vector<Value> args;
    if (call.args) flatten_args(call.args.get(), args);

    if (call.name == "echo") {
        for (auto& v : args) std::cout << v.toString();
        std::cout << std::endl;
        evalStack.push_back(Value());
        return;
    }

    auto func = currentEnv->getFunction(call.name);
    if (!func)
        throw std::runtime_error("Undefined function: " + call.name);

    callFunction(*func, args);
}

void Interpreter::flatten_args(ast::Expr* node, std::vector<Value>& out) {
    if (!node) return;   // empty args (())
    
    if (auto* bin = dynamic_cast<ast::BinaryExpr*>(node);
        bin && bin->op == ast::BinaryOperator::Comma)
    {
        flatten_args(bin->left.get(), out);
        flatten_args(bin->right.get(), out);
        return;
    }
    
    // not a Comma at the top level — evaluate as expression
    evaluate(*node);
    out.push_back(popValue());
}

void Interpreter::flatten_params(ast::Expr* node, std::vector<ast::VarDeclExpr*>& out) {
    if (!node) return;
    
    if (auto* bin = dynamic_cast<ast::BinaryExpr*>(node);
        bin && bin->op == ast::BinaryOperator::Comma)
    {
        flatten_params(bin->left.get(), out);
        flatten_params(bin->right.get(), out);
        return;
    }
    
    if (auto* decl = dynamic_cast<ast::VarDeclExpr*>(node)) {
        out.push_back(decl);
        return;
    }
    
    throw std::runtime_error("Invalid parameter declaration");
}

} // namespace interpreter
