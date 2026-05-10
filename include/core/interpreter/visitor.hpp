#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <stdexcept>
#include <optional>

#include "ast/ast.hpp"

namespace interpreter {
struct Value {
    enum class Type {
        Number,
        String,
        Boolean,
        Void
    };

    enum class NumberKind {
        Float,
        Int,
    };

    Type type;
    NumberKind numberKind;
    std::variant<double, std::string, bool> data;

    Value() : type(Type::Void), numberKind(NumberKind::Float), data(0.0) {}
    Value(double num) : type(Type::Number), numberKind(NumberKind::Float), data(num) {}
    Value(double num, NumberKind kind) : type(Type::Number), numberKind(kind), data(num) {}
    Value(const std::string& str) : type(Type::String), numberKind(NumberKind::Float), data(str) {}
    Value(bool boolean) : type(Type::Boolean), numberKind(NumberKind::Float), data(boolean) {}

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
    std::vector<ast::VarDeclExpr*> params;
    ast::Stmt* body;
    std::string returnType;

    using Ref = std::reference_wrapper<Function>;
};

class Environment {
public:
    using Ref = std::reference_wrapper<Environment>;

    Environment() : parent(std::nullopt) {}
    Environment(Environment& parent) : parent(parent) {}

    void defineVariable(const std::string& name, const Value& value);
    void defineFunction(const std::string& name, Function func);

    Value getVariable(const std::string& name);
    void setVariable(const std::string& name, const Value& value);

    std::optional<Function::Ref> getFunction(const std::string& name);
    std::shared_ptr<Environment> createChild();

    // remove copy and move constructors and assignment operators
    Environment(const Environment&) = delete;
    Environment& operator=(const Environment&) = delete;
    Environment(Environment&&) = delete;
    Environment& operator=(Environment&&) = delete;

private:
    std::optional<Environment::Ref> parent;
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, Function> functions;

};

class Visitor {
public:
    virtual ~Visitor() = default;
    
    virtual void visit(ast::BinaryExpr& expr) = 0;
    virtual void visit(ast::UnaryExpr& expr) = 0;
    virtual void visit(ast::VarDeclExpr& stmt) = 0;
    virtual void visit(ast::LiteralNumber& expr) = 0;
    virtual void visit(ast::LiteralString& expr) = 0;
    virtual void visit(ast::Identifier& expr) = 0;
    virtual void visit(ast::ExpressionStmt& stmt) = 0;
    virtual void visit(ast::ReturnStmt& stmt) = 0;
    virtual void visit(ast::BlockStmt& stmt) = 0;
    virtual void visit(ast::IfStmt& stmt) = 0;
    virtual void visit(ast::ForStmt& stmt) = 0;
    virtual void visit(ast::FunctionDecl& stmt) = 0;
    virtual void visit(ast::FunctionCall& expr) = 0;

};

} // namespace interpreter
