#pragma once

#include "interpreter/visitor.hpp"
#include "ast/ast.hpp"
#include <vector>
#include <memory>

namespace interpreter {
class Interpreter : public Visitor {
public:
    Interpreter() : hasReturned(false) {
        globalEnv = std::make_shared<Environment>();
        currentEnv = globalEnv;
    }

    void interpret(const std::vector<ast::Statement>& statements);
    Value getResult() const;

    void evaluate(ast::Expr& expr);
    Value popValue();
    void applyBinaryOperator(ast::BinaryOperator op);
    void applyUnaryOperator(ast::UnaryOperator op, ast::Expr& operand);

    /* expressions */
    void visit(ast::BinaryExpr& expr) override;
    void visit(ast::UnaryExpr& expr) override;
    void visit(ast::VarDeclExpr& expr) override;
    void visit(ast::LiteralNumber& expr) override;
    void visit(ast::LiteralString& expr) override;
    void visit(ast::Identifier& expr) override;
    void visit(ast::FunctionCall& expr) override;

    /* statements */
    void visit(ast::ExpressionStmt& stmt) override;
    void visit(ast::ReturnStmt& stmt) override;
    void visit(ast::BlockStmt& stmt) override;
    void visit(ast::IfStmt& stmt) override;
    void visit(ast::ForStmt& stmt) override;
    void visit(ast::FunctionDecl& stmt) override;

private:
    std::shared_ptr<Environment> globalEnv;
    std::shared_ptr<Environment> currentEnv;
    
    Value returnValue;
    bool hasReturned;

    std::vector<Value> evalStack;

    void callFunction(Function& func, const std::vector<Value>& args = {});

    void flatten_args(ast::Expr* node, std::vector<Value>& out);
    void flatten_params(ast::Expr* node, std::vector<ast::VarDeclExpr*>& out);
};

} // namespace interpreter