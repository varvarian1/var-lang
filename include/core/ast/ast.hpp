#pragma once

#include <memory>
#include <vector>
#include <string>
#include <utility>

namespace interpreter {
class Visitor;
}

namespace ast {
enum class Op {
    Plus, Minus, Multiply, Divide
};

enum class ComparisonOp {
    Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual
};

struct Expr {
    Expr() {}
    virtual ~Expr() = default;
    virtual void accept(interpreter::Visitor& visitor) = 0;
};

struct BinaryExpr : Expr {
    BinaryExpr(std::unique_ptr<Expr> left, Op op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    std::unique_ptr<Expr> left;
    Op op;
    std::unique_ptr<Expr> right;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct ComparisonExpr : Expr {
    ComparisonExpr(std::unique_ptr<Expr> left, ComparisonOp op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    std::unique_ptr<Expr> left;
    ComparisonOp op;
    std::unique_ptr<Expr> right;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct LiteralNumber : Expr {
    LiteralNumber(double value) : value(value) {}
    double value;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct LiteralString : Expr {
    LiteralString(const std::string& str) : value(str) {}
    std::string value;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct Identifier : Expr {
    Identifier(const std::string& name) : name(name) {}
    std::string name;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct FunctionCall : Expr {
    FunctionCall(std::string name, std::vector<std::unique_ptr<Expr>> args)
        : name(name), args(std::move(args)) {}
    
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct Stmt {
    Stmt() {}
    virtual ~Stmt() = default;
    virtual void accept(interpreter::Visitor& visitor) = 0;
};

struct ExpressionStmt : Stmt {
    ExpressionStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
    std::unique_ptr<Expr> expr;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct VariableDeclarationStmt : Stmt {
    VariableDeclarationStmt(const std::string& type, const std::string& name, std::unique_ptr<Expr> initializer)
        : type(type), name(name), initializer(std::move(initializer)) {}

    std::string type;
    std::string name;
    std::unique_ptr<Expr> initializer;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct AssignStmt : Stmt {
    AssignStmt(const std::string& name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}

    std::string name;
    std::unique_ptr<Expr> value;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct BlockStmt : Stmt {
    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {}

    std::vector<std::unique_ptr<Stmt>> statements;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct IfStmt : Stmt {
    IfStmt(std::unique_ptr<Expr> condition, 
           std::unique_ptr<Stmt> thenBranch, 
           std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), 
          thenBranch(std::move(thenBranch)), 
          elseBranch(std::move(elseBranch)) {}

    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct ForStmt : Stmt {
    ForStmt(std::unique_ptr<Stmt> initializer,
            std::unique_ptr<Expr> condition,
            std::unique_ptr<Stmt> increment,
            std::unique_ptr<Stmt> body)
        : initializer(std::move(initializer)),
          condition(std::move(condition)),
          increment(std::move(increment)),
          body(std::move(body)) {}

    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> increment;
    std::unique_ptr<Stmt> body;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct FunctionStmt : Stmt {
    FunctionStmt(const std::string& name, 
                 std::vector<std::pair<std::string, std::string>> params,
                 const std::string& returnType,
                 std::unique_ptr<Stmt> body)
        : name(name), params(params), returnType(returnType), body(std::move(body)) {}

    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::string returnType;
    std::unique_ptr<Stmt> body;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct ReturnStmt : Stmt {
    ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
    std::unique_ptr<Expr> value;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct EchoStmt : Stmt {
    EchoStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
    std::unique_ptr<Expr> expr;
    
    void accept(interpreter::Visitor& visitor) override;
};

} // namespace ast
