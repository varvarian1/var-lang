#include "interpreter/visitor.hpp"
#include "ast/ast.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace interpreter {
bool Value::isNumber() const { 
    return type == Type::Number;
}

bool Value::isString() const {
    return type == Type::String;
}

bool Value::isBoolean() const {
    return type == Type::Boolean;
}

bool Value::isVoid() const {
    return type == Type::Void;
}

double Value::asNumber() const {
    if (!isNumber())
        throw std::runtime_error("Value is not a number");

    return std::get<double>(data);
}

std::string Value::asString() const {
    if (!isString())
        throw std::runtime_error("Value is not a string");

    return std::get<std::string>(data);
}

bool Value::asBoolean() const {
    if (!isBoolean())
        throw std::runtime_error("Value is not a boolean");

    return std::get<bool>(data);
}

std::string Value::toString() const {
    switch (type) {
        case Type::Number:
            if (numberKind == NumberKind::Int) {
                return std::to_string(static_cast<long long>(std::get<double>(data)));
            } else {
                std::ostringstream oss;
                oss.setf(std::ios::fmtflags(0), std::ios::floatfield);
                oss << std::setprecision(15) << std::get<double>(data);
                return oss.str();
            }

        case Type::String:
            return std::get<std::string>(data);

        case Type::Boolean:
            return std::get<bool>(data) ? "true" : "false";

        case Type::Void:
            return "void";
    }

    return "";
}

void Environment::defineVariable(const std::string& name, const Value& value) {
    variables[name] = value;
}

void Environment::defineFunction(const std::string& name, Function func) {
    functions[name] = std::move(func);
}

Value Environment::getVariable(const std::string& name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    if (parent) {
        return parent->get().getVariable(name);
    }

    throw std::runtime_error("Undefined variable: " + name);
}

void Environment::setVariable(const std::string& name, const Value& value) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        it->second = value;
        return;
    }
    if (parent) {
        parent->get().setVariable(name, value);
        return;
    }

    throw std::runtime_error("Undefined variable: " + name);
}

std::optional<Function::Ref> Environment::getFunction(const std::string& name) {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second;
    }
    if (parent) {
        std::optional<Function::Ref> func = parent->get().getFunction(name);
        if (func) {
            return func;
        }
    }
    
    return std::nullopt;
}

std::shared_ptr<Environment> Environment::createChild() {
    return std::make_shared<Environment>(*this);
}

} // namespace interpreter

namespace ast {
void BinaryExpr::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void UnaryExpr::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void VarDeclExpr::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void LiteralNumber::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void LiteralString::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void Identifier::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void ExpressionStmt::accept(interpreter::Visitor& visitor) {
    visitor.visit(*this);
}

void ReturnStmt::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void BlockStmt::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void IfStmt::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void ForStmt::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void FunctionDecl::accept(interpreter::Visitor& visitor) { 
    visitor.visit(*this); 
}

void ast::FunctionCall::accept(interpreter::Visitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
