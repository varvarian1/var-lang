#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <stdexcept>

#include "ast/ast.hpp"

namespace interpreter {
struct Value {
    enum class Type {
        Number,
        String,
        Boolean,
        Void
    };

    Type type;
    std::variant<double, std::string, bool> data;

    Value() : type(Type::Void), data(0.0) {}
    Value(double num) : type(Type::Number), data(num) {}
    Value(const std::string& str) : type(Type::String), data(str) {}
    Value(bool boolean) : type(Type::Boolean), data(boolean) {}

    bool isNumber() const;
    bool isString() const;
    bool isBoolean() const;
    bool isVoid() const;

    double asNumber() const;
    std::string asString() const;
    bool asBoolean() const;

    std::string toString() const;
};

struct Function {
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    ast::Statement body;
    std::string returnType;
};

class Environment {
public:
    Environment(Environment* parent = nullptr) : parent(parent) {}

    void defineVariable(const std::string& name, const Value& value);
    void defineFunction(const std::string& name, Function func);

    Value getVariable(const std::string& name);
    void setVariable(const std::string& name, const Value& value);

    Function* getFunction(const std::string& name);
    Environment* createChild();

private:
    Environment* parent;
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, Function> functions;

};

class Visitor {
public:
    virtual ~Visitor() = default;
    
    virtual void visit(ast::BinaryExpr& expr) = 0;
    virtual void visit(ast::ComparisonExpr& expr) = 0;
    virtual void visit(ast::LiteralNumber& expr) = 0;
    virtual void visit(ast::LiteralString& expr) = 0;
    virtual void visit(ast::Identifier& expr) = 0;
    virtual void visit(ast::VariableDeclarationStmt& stmt) = 0;
    virtual void visit(ast::ExpressionStmt& stmt) = 0;
    virtual void visit(ast::AssignStmt& stmt) = 0;
    virtual void visit(ast::BlockStmt& stmt) = 0;
    virtual void visit(ast::IfStmt& stmt) = 0;
    virtual void visit(ast::ForStmt& stmt) = 0;
    virtual void visit(ast::FunctionStmt& stmt) = 0;
    virtual void visit(ast::FunctionCall& expr) = 0;
    virtual void visit(ast::ReturnStmt& stmt) = 0;
    virtual void visit(ast::EchoStmt& stmt) = 0;
};

} // namespace interpreter
