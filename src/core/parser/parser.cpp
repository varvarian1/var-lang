#include <iomanip>
#include <type_traits>

#include "parser/parser.hpp"

namespace parser {
std::vector<std::unique_ptr<ast::Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<ast::Stmt>> statements;

    while (currentToken.type != Token::Type::EndToken) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
        else {
            break;
        }
    }

    return statements;
}

std::unique_ptr<ast::Stmt> Parser::declaration() {
    if (currentToken.type == Token::Type::EndToken) {
        return nullptr;
    }

    if (match({Token::Keyword::LET})) {
        return letDeclaration();
    }
    else if (check(Token::Type::Identifier) && peekNext().type == Token::Type::Identifier) {
        return variableDeclaration();
    }
    else if (match({Token::Keyword::FUNCTION})) {
        return functionDeclaration();
    }
    else if (match({Token::Keyword::IF})) {
        return ifStmt();
    }
    else if (match({Token::Keyword::RETURN})) {
        return returnStmt();
    }
    else if (match({Token::Keyword::ECHO})) {
        return echoStmt();
    }
    else if (match({Token::Keyword::FOR})) {
        return forStmt();
    }
    else if (check(Token::Type::LeftBrace)) {
        return block();
    }
    
    throw std::runtime_error("Invalid declaration! Current token: " + currentToken.text);
}

std::unique_ptr<ast::Stmt> Parser::letDeclaration() {
    std::string variableName = currentToken.text;
    eat(Token::Type::Identifier);
    
    eat(Token::Type::Colon);
    
    std::string variableType = parseType();
    
    std::unique_ptr<ast::Expr> initializer = nullptr;
    if (match({Token::Type::Equal})) {
        initializer = expression();
    }
    
    eat(Token::Type::Semicolon);
    
    return std::make_unique<ast::VariableDeclarationStmt>(variableType, variableName, std::move(initializer));
}

std::unique_ptr<ast::Stmt> Parser::functionDeclaration() {
    std::string functionName = currentToken.text;
    eat(Token::Type::Identifier);
    
    eat(Token::Type::LeftParenthesis);
    std::vector<std::pair<std::string, std::string>> params;
    
    if (!check(Token::Type::RightParenthesis)) {
        do {
            std::string paramType = parseType();
            std::string paramName = currentToken.text;

            eat(Token::Type::Identifier);
            params.push_back({paramType, paramName});
        }
        while (match({Token::Type::Comma}));
    }
    
    eat(Token::Type::RightParenthesis);
    
    std::string returnType = "void";
    if (match({Token::Keyword::AS})) {
        returnType = parseType();
    }
    
    auto body = block();
    return std::make_unique<ast::FunctionStmt>(functionName, params, returnType, std::move(body));
}

std::string Parser::parseType() {
    if (currentToken.type == Token::Type::Keyword) {
        std::string type = currentToken.text;
        nextToken();

        return type;
    }

    throw std::runtime_error("Expected type");
}

std::unique_ptr<ast::Stmt> Parser::variableDeclaration() {
    std::string variableType = parseType();
    
    std::string variableName = currentToken.text;
    eat(Token::Type::Identifier);
    
    std::unique_ptr<ast::Expr> initializer = nullptr;
    if (match({Token::Type::Equal})) {
        initializer = expression();
    }
    eat(Token::Type::Semicolon);

    return std::make_unique<ast::VariableDeclarationStmt>(variableType, variableName, std::move(initializer));
}

std::unique_ptr<ast::Stmt> Parser::block() {
    std::vector<std::unique_ptr<ast::Stmt>> statements;

    if (currentToken.type != Token::Type::LeftBrace) {
        throw std::runtime_error("Expected '{' at beginning of block");
    }
    
    eat(Token::Type::LeftBrace);

    while (!check(Token::Type::RightBrace) && !check(Token::Type::EndToken)) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
        else {
            break;
        }
    }
    
    if (check(Token::Type::RightBrace)) {
        eat(Token::Type::RightBrace);
    }
    else if (currentToken.type != Token::Type::EndToken) {
        throw std::runtime_error("Expected '}' but got: " + currentToken.text);
    }

    return std::make_unique<ast::BlockStmt>(std::move(statements));
}

std::unique_ptr<ast::Expr> Parser::expression() {
    return comparison();
}

std::unique_ptr<ast::Expr> Parser::term() {
    auto left = factor();
    
    while (isPlus(currentToken) || isMinus(currentToken)) {
        ast::Op op;
        switch (currentToken.type) {
            case Token::Type::Plus:
                op = ast::Op::Plus;
                break;
            case Token::Type::Minus:
                op = ast::Op::Minus;
                break;
            default:
                throw std::runtime_error("Expect '+' or '-'");
        }
        
        nextToken();
        auto right = factor();
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

bool Parser::isPlus(const Token& token) const {
    return token.type == Token::Type::Plus;
}

bool Parser::isMinus(const Token& token) const {
    return token.type == Token::Type::Minus;
}

std::unique_ptr<ast::Expr> Parser::comparison() {
    auto left = term();
    
    while (isComparison(currentToken)) {
        ast::ComparisonOp op;
        switch (currentToken.type) {
            case Token::Type::Equal:
                op = ast::ComparisonOp::Equal;
                break;
            case Token::Type::NotEqual:
                op = ast::ComparisonOp::NotEqual;
                break;
            case Token::Type::Less:
                op = ast::ComparisonOp::Less;
                break;
            case Token::Type::LessEqual:
                op = ast::ComparisonOp::LessEqual;
                break;
            case Token::Type::Greater:
                op = ast::ComparisonOp::Greater;
                break;
            case Token::Type::GreaterEqual:
                op = ast::ComparisonOp::GreaterEqual;
                break;
            default:
                throw std::runtime_error("Expect comparison operator");
        }
        
        nextToken();
        auto right = term();
        left = std::make_unique<ast::ComparisonExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

bool Parser::isComparison(const Token& token) const {
    return token.type == Token::Type::Equal ||
           token.type == Token::Type::NotEqual ||
           token.type == Token::Type::Less ||
           token.type == Token::Type::LessEqual ||
           token.type == Token::Type::Greater ||
           token.type == Token::Type::GreaterEqual;
}
 
std::unique_ptr<ast::Expr> Parser::factor() {
    if (currentToken.type == Token::Type::Number)
        return parseNumber();

    else if (currentToken.type == Token::Type::String)
        return parseString();

    else if (currentToken.type == Token::Type::Identifier) {
        std::string name = currentToken.text;
        nextToken();
        
        if (check(Token::Type::LeftParenthesis)) {
            eat(Token::Type::LeftParenthesis);
            std::vector<std::unique_ptr<ast::Expr>> args;
            
            if (!check(Token::Type::RightParenthesis)) {
                do {
                    args.push_back(expression());
                } while (match({Token::Type::Comma}));
            }
            
            eat(Token::Type::RightParenthesis);
            return std::make_unique<ast::FunctionCall>(name, std::move(args));
        }
        
        return std::make_unique<ast::Identifier>(name);
    }

    else if (currentToken.type == Token::Type::LeftParenthesis)
        return parseParenthesizedExpr();
    
    throw std::runtime_error(
        "Invalid factor: expected number, variable or '(' but got '" + currentToken.text + "'");
}

std::unique_ptr<ast::Expr> Parser::parseNumber() {
    double value = std::stod(currentToken.text);
    nextToken();

    return std::make_unique<ast::LiteralNumber>(value);
}

std::unique_ptr<ast::Expr> Parser::parseString() {
    std::string str = currentToken.text;
    nextToken();

    return std::make_unique<ast::LiteralString>(str);
}

std::unique_ptr<ast::Expr> Parser::parseParenthesizedExpr() {
    eat(Token::Type::LeftParenthesis);
    auto expr = expression();
    eat(Token::Type::RightParenthesis);

    return expr; 
}

std::unique_ptr<ast::Stmt> Parser::assignStmt() {
    std::string variableName = currentToken.text;
    eat(Token::Type::Identifier);
    eat(Token::Type::Equal);

    auto expr = expression();
    eat(Token::Type::Semicolon);

    return std::make_unique<ast::AssignStmt>(variableName, std::move(expr));
}

std::unique_ptr<ast::Stmt> Parser::statement() {
    if (match({Token::Keyword::ECHO})) {
        return echoStmt();
    }
    else if (match({Token::Keyword::RETURN})) {
        return returnStmt();
    }
    else if (check(Token::Type::Identifier)) {
        return assignStmt();
    }
    
    throw std::runtime_error("Invalid statement");
}

std::unique_ptr<ast::Stmt> Parser::ifStmt() {
    eat(Token::Type::LeftParenthesis);
    auto condition = expression();
    eat(Token::Type::RightParenthesis);
    
    auto thenBranch = block();
    std::unique_ptr<ast::Stmt> elseBranch = nullptr;
    
    if (currentToken.type == Token::Type::Keyword) {
        auto& [_, k_meta] = program;
        Token::Keyword* keyword = k_meta.get(tokenIndex);
        if (keyword && *keyword == Token::Keyword::ELSE) {
            nextToken();
            elseBranch = block();
        }
    }
    
    return std::make_unique<ast::IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<ast::Stmt> Parser::forStmt() {
    eat(Token::Type::LeftParenthesis);
    
    std::unique_ptr<ast::Stmt> initializer = nullptr;
    std::unique_ptr<ast::Expr> condition = nullptr;
    std::unique_ptr<ast::Stmt> increment = nullptr;
    
    if (!check(Token::Type::Semicolon)) {
        if (match({Token::Keyword::LET})) {
            std::string variableName = currentToken.text;
            eat(Token::Type::Identifier);
            
            eat(Token::Type::Colon);
            
            std::string variableType = parseType();
            
            std::unique_ptr<ast::Expr> initialValue = nullptr;
            if (match({Token::Type::Equal})) {
                initialValue = expression();
            }
            
            initializer = std::make_unique<ast::VariableDeclarationStmt>(
                variableType, variableName, std::move(initialValue)
            );
        }
        else if (check(Token::Type::Identifier) && peekNext().type == Token::Type::Identifier) {
            initializer = variableDeclaration();
        }
        else if (check(Token::Type::Identifier)) {
            initializer = assignStmt();
        }
    }
    
    eat(Token::Type::Semicolon);
    
    if (!check(Token::Type::Semicolon)) {
        condition = expression();
    }
    
    eat(Token::Type::Semicolon);
    
    if (!check(Token::Type::RightParenthesis)) {
        if (check(Token::Type::Identifier)) {
            std::string varName = currentToken.text;
            eat(Token::Type::Identifier);
            
            if (match({Token::Type::Plus, Token::Type::Plus})) {
                auto right = std::make_unique<ast::BinaryExpr>(
                    std::make_unique<ast::Identifier>(varName),
                    ast::Op::Plus,
                    std::make_unique<ast::LiteralNumber>(1.0)
                );
                increment = std::make_unique<ast::AssignStmt>(varName, std::move(right));
            }
            else if (match({Token::Type::Equal})) {
                auto expr = expression();
                increment = std::make_unique<ast::AssignStmt>(varName, std::move(expr));
            }
            else {
                throw std::runtime_error("Expected '++' or '=' in for loop increment");
            }
        }
    }
    
    eat(Token::Type::RightParenthesis);
    
    std::unique_ptr<ast::Stmt> body;
    if (check(Token::Type::LeftBrace)) {
        body = block();
    }
    else {
        body = statement();
    }
    
    return std::make_unique<ast::ForStmt>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}

std::unique_ptr<ast::Stmt> Parser::returnStmt() {
    auto expr = expression();
    eat(Token::Type::Semicolon);

    return std::make_unique<ast::ReturnStmt>(std::move(expr));
}

std::unique_ptr<ast::Stmt> Parser::echoStmt() {
    auto expr = expression();
    eat(Token::Type::Semicolon);

    return std::make_unique<ast::EchoStmt>(std::move(expr));
}

bool Parser::check(Token::Type type) const {
    if (currentToken.type == Token::Type::EndToken) 
        return false;
    
    return currentToken.type == type;
}

Token Parser::nextToken() {
    if (tokenIndex + 1 < tokensVector.size()) {
        tokenIndex++;
        currentToken = tokensVector[tokenIndex];
    } 
    else {
        currentToken = {Token::Type::EndToken, ""};
    }

    return currentToken;
}

Token Parser::peekNext() const {
    if (tokenIndex + 1 < tokensVector.size()) {
        return tokensVector[tokenIndex + 1];
    }

    return {Token::Type::EndToken, ""};
}

} // namespace parser
