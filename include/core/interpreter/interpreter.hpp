#pragma once

#include "interpreter/visitor.hpp"
#include "ast/ast.hpp"
#include <vector>
#include <memory>

namespace interpreter {
class Interpreter : public Visitor {
public:
    Interpreter() : hasReturned(false) {
        globalEnv = new Environment();
        currentEnv = globalEnv;
    }

    ~Interpreter() {
        delete globalEnv;
    }

    void interpret(const std::vector<std::unique_ptr<ast::Stmt>>& statements);
    Value getResult() const;

    void evaluate(ast::Expr& expr);
    Value popValue();
    void applyBinaryOp(ast::Op op);
    void applyComparisonOp(ast::ComparisonOp op);

    /* expressions */
    void visit(ast::BinaryExpr& expr) override;
    void visit(ast::ComparisonExpr& expr) override;
    void visit(ast::LiteralNumber& expr) override;
    void visit(ast::LiteralString& expr) override;
    void visit(ast::Identifier& expr) override;

    /* statements */
    void visit(ast::VariableDeclarationStmt& stmt) override;
    void visit(ast::ExpressionStmt& stmt) override;
    void visit(ast::AssignStmt& stmt) override;
    void visit(ast::BlockStmt& stmt) override;
    void visit(ast::IfStmt& stmt) override;
    void visit(ast::ForStmt& stmt) override;
    void visit(ast::FunctionStmt& stmt) override;
    void visit(ast::FunctionCall& expr) override;
    void visit(ast::ReturnStmt& stmt) override;
    void visit(ast::EchoStmt& stmt) override;

private:
    Environment* globalEnv;
    Environment* currentEnv;
    
    Value returnValue;
    bool hasReturned;

    std::vector<Value> evalStack;

    void callFunction(Function* func, const std::vector<Value>& args = {});
};

} // namespace interpreter