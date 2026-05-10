#pragma once

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>
#include <string>
#include <utility>
#include <map>

#include "lexer/token.hpp"
namespace interpreter {
class Visitor;
}

namespace ast {

enum class Precedence {
    None       = 0,
    Comma,
    Assign,
    Equality,       // == !=
    Relational,     // < > <= >=
    Additive,       // + -
    Multiply,       // * /
    Unary,
    Postfix,
    Primary,
};

inline bool operator<(Precedence a, Precedence b) noexcept {
    return static_cast<int>(a) < static_cast<int>(b);
}

inline bool operator>=(Precedence a, Precedence b) noexcept {
    return static_cast<int>(a) >= static_cast<int>(b);
}

inline Precedence next(Precedence p) noexcept {
    return static_cast<Precedence>(static_cast<int>(p) + 1);
}

#define BINARY_OP   \
    X(Unknown, 0)   \
    X(Assign, 1)    \
    X(Add, 1)       \
    X(Subtract, 1)  \
    X(Multiply, 1)  \
    X(Divide, 1)    \
    X(Equal, 1)     \
    X(NotEqual, 2)  \
    X(Less, 1)      \
    X(LessEqual, 2) \
    X(Greater, 1)   \
    X(GreaterEqual, 2) \
    X(Comma, 1)

enum class BinaryOperator {
    #define X(name, _) name,
            BINARY_OP
    #undef X
};

#define UNARY_OP        \
    X(Unknown)          \
    X(UnaryMinus)       \
    X(UnaryPlus)        \
    X(PreIncrement)     \
    X(PreDecrement)     \
    X(PostIncrement)    \
    X(PostDecrement)

enum class UnaryOperator {
    #define X(name) name,
            UNARY_OP
    #undef X
};

struct BinaryOperatorInfo {
    Precedence precedence;
    int symbols;
    bool left_associative;
};

static std::map<BinaryOperator, BinaryOperatorInfo> binary_operator_info = {
    { BinaryOperator::Unknown,      { Precedence::None,         0, false } },

    { BinaryOperator::Assign,       { Precedence::Assign,       1, false } },

    { BinaryOperator::Equal,        { Precedence::Equality,     1, true } },
    { BinaryOperator::NotEqual,     { Precedence::Equality,     2, true } },

    { BinaryOperator::Less,         { Precedence::Relational,   1, true } },
    { BinaryOperator::Greater,      { Precedence::Relational,   1, true } },
    { BinaryOperator::LessEqual,    { Precedence::Relational,   2, true } },
    { BinaryOperator::GreaterEqual, { Precedence::Relational,   2, true } },

    { BinaryOperator::Add,          { Precedence::Additive,     1, true } },
    { BinaryOperator::Subtract,     { Precedence::Additive,     1, true } },

    { BinaryOperator::Multiply,     { Precedence::Multiply,     1, true } },
    { BinaryOperator::Divide,       { Precedence::Multiply,     1, true } },

    { BinaryOperator::Comma,        { Precedence::Comma,        1, true } },
};

struct UnaryOperatorInfo {
    Precedence precedence;
    int symbols;
    bool is_prefix;
};

static std::map<UnaryOperator, UnaryOperatorInfo> unary_operator_info = {
    { UnaryOperator::Unknown,       { Precedence::None,         0, false } },

    { UnaryOperator::UnaryMinus,    { Precedence::Unary,        1, false } },
    { UnaryOperator::UnaryPlus,     { Precedence::Unary,        1, false } },

    { UnaryOperator::PreIncrement,  { Precedence::Unary,        2, false } },
    { UnaryOperator::PreDecrement,  { Precedence::Unary,        2, false } },

    { UnaryOperator::PostIncrement, { Precedence::Postfix,      2, false } },
    { UnaryOperator::PostDecrement, { Precedence::Postfix,      2, false } },
};

struct Expr {
    Expr() {}
    virtual ~Expr() = default;
    virtual void accept(interpreter::Visitor& visitor) = 0;
};

using Expression = std::unique_ptr<ast::Expr>;

struct BinaryExpr : Expr {
    BinaryExpr(std::unique_ptr<Expr> left, BinaryOperator op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    std::unique_ptr<Expr> left;
    BinaryOperator op;
    std::unique_ptr<Expr> right;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct UnaryExpr : Expr {
    UnaryExpr(std::unique_ptr<Expr> left, UnaryOperator op, bool is_prefix)
        : is_prefix(is_prefix), left(std::move(left)), op(op) {}

    bool is_prefix;
    std::unique_ptr<Expr> left;
    UnaryOperator op;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct VarDeclExpr : Expr {
    VarDeclExpr(const std::string& type, const std::string& name)
        : type(type), name(name) {}

    std::string type;
    std::string name;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct LiteralNumber : Expr {
    enum class Type {
        INT,
        FLOAT,
    };

    LiteralNumber(double value, Type type)
        : value(value), type(type) {}

    double value;
    Type type;

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
    FunctionCall(std::string name, std::unique_ptr<Expr> args)
        : name(name), args(std::move(args)) {}
    
    std::string name;
    std::unique_ptr<Expr> args;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct Stmt {
    Stmt() {}
    virtual ~Stmt() = default;
    virtual void accept(interpreter::Visitor& visitor) = 0;
};

using Statement = std::unique_ptr<ast::Stmt>;

struct ExpressionStmt : Stmt {
    ExpressionStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
    std::unique_ptr<Expr> expr;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct ReturnStmt : Stmt {
    ReturnStmt(std::unique_ptr<Expr> value)
        : value(std::move(value)) {}

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
    ForStmt(std::unique_ptr<Expr> initializer,
            std::unique_ptr<Expr> condition,
            std::unique_ptr<Expr> increment,
            std::unique_ptr<Stmt> body)
        : initializer(std::move(initializer)),
          condition(std::move(condition)),
          increment(std::move(increment)),
          body(std::move(body)) {}

    std::unique_ptr<Expr> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<Stmt> body;
    
    void accept(interpreter::Visitor& visitor) override;
};

struct FunctionDecl : Stmt {
    FunctionDecl(const std::string& name, 
                 std::unique_ptr<Expr> params,
                 const std::string& returnType,
                 std::unique_ptr<Stmt> body)
        : name(name), params(std::move(params)), returnType(returnType), body(std::move(body)) {}

    std::string name;
    std::unique_ptr<Expr> params;
    std::string returnType;
    std::unique_ptr<Stmt> body;
    
    void accept(interpreter::Visitor& visitor) override;
};

} // namespace ast
