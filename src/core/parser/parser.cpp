#include <iomanip>
#include <type_traits>

#include "parser/parser.hpp"
namespace parser {
std::vector<ast::Statement> Parser::parse() {
    std::vector<ast::Statement> statements;

    while (true) {
        auto stmt = declaration();
        if (stmt.ok()) {
            statements.push_back(std::move(stmt.value()));
        } else {
            auto &[ message, token ] = stmt.error();
            std::cerr << token.pos.filename << ":" << token.pos.line << ":" << token.pos.column << " | " << message << std::endl;

            skipUntil(Token::Type::Semicolon);
            nextToken();
        }

        if (currentToken.type == Token::Type::EndToken) {
            break;
        }
    }

    return statements;
}

ParseResult<ast::Statement> Parser::declaration() {
    if (currentToken.type == Token::Type::EndToken) {
        return ErrorMeta{"Unexpected end of input", currentToken};
    }

    if (match({Keyword::LET})) {
        return letDeclaration();
    }
    else if (check(Token::Type::Identifier)) {
        auto next = peekNext();
        if (next && next->type == Token::Type::Identifier) {
            return variableDeclaration();
        } else {
            std::string name = currentToken.text;
            size_t savedPosition = tokenIndex;
            Token savedToken = currentToken;

            nextToken();

            if (check(Token::Type::LeftParenthesis)) {
                tokenIndex = savedPosition;
                currentToken = savedToken;

                return functionCallStmt();
            }
            else {
                tokenIndex = savedPosition;
                currentToken = savedToken;

                return ErrorMeta{"Unexpected identifier: " + currentToken.text, currentToken};
            }
        }
    }
    else if (match({Keyword::FUNCTION})) {
        return functionDeclaration();
    }
    else if (match({Keyword::IF})) {
        return ifStmt();
    }
    else if (match({Keyword::RETURN})) {
        return returnStmt();
    }
    else if (match({Keyword::ECHO})) {
        return echoStmt();
    }
    else if (match({Keyword::FOR})) {
        return forStmt();
    }
    else if (check(Token::Type::LeftBrace)) {
        return block();
    }
    
    return ErrorMeta{"Unexpected token: " + currentToken.text, currentToken};
}

ParseResult<ast::Statement> Parser::letDeclaration() {
    std::string variableName = currentToken.text;
    TRY(eat(Token::Type::Identifier));
    
    TRY(eat(Token::Type::Colon));
    
    std::string variableType = TRY(parseType());

    ast::Expression initializer = nullptr;
    if (match({Token::Type::Equal})) {
        initializer = TRY(expression());
    }
    
    TRY(eat(Token::Type::Semicolon));
    
    return make_ok<ast::VariableDeclarationStmt>(variableType, variableName, std::move(initializer));
}

ParseResult<ast::Statement> Parser::functionDeclaration() {
    std::string functionName = currentToken.text;
    TRY(eat(Token::Type::Identifier));
    
    TRY(eat(Token::Type::LeftParenthesis));
    std::vector<std::pair<std::string, std::string>> params;
    
    if (!check(Token::Type::RightParenthesis)) {
        do {
            std::string paramType = TRY(parseType());
            std::string paramName = currentToken.text;

            TRY(eat(Token::Type::Identifier));
            params.push_back({paramType, paramName});
        }
        while (match({Token::Type::Comma}));
    }
    
    TRY(eat(Token::Type::RightParenthesis));
    
    std::string returnType = "void";
    if (match({Keyword::AS})) {
        returnType = TRY(parseType());
    }
    
    ast::Statement body = TRY(block());
    return make_ok<ast::FunctionStmt>(functionName, params, returnType, std::move(body));
}

ParseResult<std::string> Parser::parseType() {
    if (currentToken.type != Token::Type::Keyword) {
        std::string errorMsg = "Expected type";
        
        errorMsg += ", but got '" + currentToken.text + "'";
        
        if (currentToken.type == Token::Type::Identifier) {
            errorMsg += " (identifiers cannot be used as types)";
        }
        
        return ErrorMeta{errorMsg, currentToken};
    }
    
    std::string type = currentToken.text;
    nextToken();
    
    return type;
}

ParseResult<ast::Statement> Parser::variableDeclaration() {
    std::string variableType = TRY(parseType());
    
    std::string variableName = currentToken.text;
    TRY(eat(Token::Type::Identifier));
    
    ast::Expression initializer = nullptr;
    if (match({Token::Type::Equal})) {
        initializer = TRY(expression());
    }
    TRY(eat(Token::Type::Semicolon));

    return make_ok<ast::VariableDeclarationStmt>(variableType, variableName, std::move(initializer));
}

ParseResult<ast::Statement> Parser::block() {
    std::vector<ast::Statement> statements;

    if (currentToken.type != Token::Type::LeftBrace) {
        return ErrorMeta{"Expected '{' at beginning of block, but got '" + currentToken.text + "'", currentToken};
    }
    
    TRY(eat(Token::Type::LeftBrace));

    while (true) {
        auto stmt = declaration();
        if (stmt.ok()) {
            statements.push_back(std::move(stmt.value()));
        } else {
            auto &[ message, token ] = stmt.error();
            std::cerr << token.pos.filename << ":" << token.pos.line << ":" << token.pos.column << " | " << message << std::endl;

            skipUntil(Token::Type::Semicolon);
            nextToken();
        }

        if (check(Token::Type::RightBrace) || check(Token::Type::EndToken) || currentToken.type == Token::Type::EndToken) {
            break;
        }
    }
    
    if (check(Token::Type::RightBrace)) {
        TRY(eat(Token::Type::RightBrace));
    }
    else if (currentToken.type != Token::Type::EndToken) {
        return ErrorMeta{"Expected '}' at end of block, but got '" + currentToken.text + "'", currentToken};
    }

    return make_ok<ast::BlockStmt>(std::move(statements));
}

ParseResult<ast::Expression> Parser::expression() {
    return TRY(comparison());
}

ParseResult<ast::Expression> Parser::term() {
    ast::Expression left = TRY(factor());
    
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
                return ErrorMeta{"Expected '+' or '-' in expression, but got '" + currentToken.text + "'", currentToken};
        }
        
        nextToken();
        ast::Expression right = TRY(factor());
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

ParseResult<ast::Expression> Parser::comparison() {
    ast::Expression left = TRY(term());
    
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
                return ErrorMeta{"Expected comparison operator, but got '" + currentToken.text + "'", currentToken};
        }
        
        nextToken();
        ast::Expression right = TRY(term());
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
 
ParseResult<ast::Expression> Parser::factor() {
    if (currentToken.type == Token::Type::Number)
        return parseNumber();

    else if (currentToken.type == Token::Type::String)
        return parseString();

    else if (currentToken.type == Token::Type::Identifier) {
        std::string name = currentToken.text;
        nextToken();
        
        if (check(Token::Type::LeftParenthesis)) {
            TRY(eat(Token::Type::LeftParenthesis));
            std::vector<ast::Expression> args;
            
            if (!check(Token::Type::RightParenthesis)) {
                do {
                    args.push_back(TRY(expression()));
                } while (match({Token::Type::Comma}));
            }
            
            TRY(eat(Token::Type::RightParenthesis));
            return make_ok<ast::FunctionCall>(name, std::move(args));
        }
        else {
            return make_ok<ast::Identifier>(name);
        }
    }

    else if (currentToken.type == Token::Type::LeftParenthesis)
        return parseParenthesizedExpr();
    
    return ErrorMeta{"Invalid factor: expected number, variable or '(' but got '" + currentToken.text + "'", currentToken};
}

ParseResult<ast::Expression> Parser::parseNumber() {
    double value = std::stod(currentToken.text);
    nextToken();

    return make_ok<ast::LiteralNumber>(value);
}

ParseResult<ast::Expression> Parser::parseString() {
    std::string str = currentToken.text;
    nextToken();

    return make_ok<ast::LiteralString>(str);
}

ParseResult<ast::Expression> Parser::parseParenthesizedExpr() {
    TRY(eat(Token::Type::LeftParenthesis));
    ast::Expression expr = TRY(expression());
    TRY(eat(Token::Type::RightParenthesis));

    return expr; 
}

ParseResult<ast::Expression> Parser::parseFunctionCall(const std::string& name) {
    TRY(eat(Token::Type::LeftParenthesis));

    std::vector<ast::Expression> arguments;
    if (!check(Token::Type::RightParenthesis)) {
        do {
            arguments.push_back(TRY(expression()));
        } while (match({Token::Type::Comma}));
    }

    TRY(eat(Token::Type::RightParenthesis));

    return make_ok<ast::FunctionCall>(name, std::move(arguments));
}

ParseResult<ast::Statement> Parser::assignStmt() {
    std::string variableName = currentToken.text;
    TRY(eat(Token::Type::Identifier));
    TRY(eat(Token::Type::Equal));

    ast::Expression expr = TRY(expression());
    TRY(eat(Token::Type::Semicolon));

    return make_ok<ast::AssignStmt>(variableName, std::move(expr));
}

ParseResult<ast::Statement> Parser::statement() {
    std::cout << "ast::Statement: current token = " << currentToken.text << std::endl;
    
    if (match({Keyword::LET})) {
        std::cout << "Match LET" << std::endl;
        return letDeclaration();
    }
    else if (match({Keyword::ECHO})) {
        std::cout << "Matched ECHO" << std::endl;
        return echoStmt();
    }
    else if (match({Keyword::RETURN})) {
        std::cout << "Matched RETURN" << std::endl;
        return returnStmt();
    }
    else if (check(Token::Type::Identifier)) {
        std::cout << "Found identifier: " << currentToken.text << std::endl;
        
        size_t savedPosition = tokenIndex;
        Token savedToken = currentToken;
        
        std::string name = currentToken.text;
        nextToken();
        std::cout << "Next token: " << currentToken.text << std::endl;
        
        if (check(Token::Type::LeftParenthesis)) {
            std::cout << "Found '(', treating as function call" << std::endl;
            tokenIndex = savedPosition;
            currentToken = savedToken;
            
            return functionCallStmt();
        }
        else {
            std::cout << "Not a function call, restoring" << std::endl;
            tokenIndex = savedPosition;
            currentToken = savedToken;
            
            return assignStmt();
        }
    }
    
    return ErrorMeta{"Unexpected token in statement: '" + currentToken.text + "'", currentToken};
}

ParseResult<ast::Statement> Parser::functionCallStmt() {
    std::string name = currentToken.text;
    TRY(eat(Token::Type::Identifier));
    
    TRY(eat(Token::Type::LeftParenthesis));
    
    std::vector<ast::Expression> arguments;
    
    if (!check(Token::Type::RightParenthesis)) {
        do {
            arguments.push_back(TRY(expression()));
        } while (match({Token::Type::Comma}));
    }
    
    TRY(eat(Token::Type::RightParenthesis));
    TRY(eat(Token::Type::Semicolon));
    
    auto functionCall = std::make_unique<ast::FunctionCall>(name, std::move(arguments));
    return make_ok<ast::ExpressionStmt>(std::move(functionCall));
}

ParseResult<ast::Statement> Parser::ifStmt() {
    TRY(eat(Token::Type::LeftParenthesis));
    auto condition = TRY(expression());
    TRY(eat(Token::Type::RightParenthesis));
    
    auto thenBranch = TRY(block());
    ast::Statement elseBranch = nullptr;
    
    if (currentToken.type == Token::Type::Keyword) {
        auto& [_, k_meta] = program;
        Keyword* keyword = k_meta.get(tokenIndex);
        if (keyword && *keyword == Keyword::ELSE) {
            nextToken();
            elseBranch = TRY(block());
        }
    }
    
    return make_ok<ast::IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

ParseResult<ast::Statement> Parser::forStmt() {
    TRY(eat(Token::Type::LeftParenthesis));
    
    ast::Statement initializer;
    ast::Expression condition = nullptr;
    ast::Statement increment = nullptr;
    
    if (!check(Token::Type::Semicolon)) {
        if (match({Keyword::LET})) {
            std::string variableName = currentToken.text;
            TRY(eat(Token::Type::Identifier));
            
            TRY(eat(Token::Type::Colon));
            
            std::string variableType = TRY(parseType());
            
            ast::Expression initialValue = nullptr;
            if (match({Token::Type::Equal})) {
                initialValue = TRY(expression());
            }
            
            initializer = std::make_unique<ast::VariableDeclarationStmt>(
                variableType, variableName, std::move(initialValue)
            );
        }
        else if (check(Token::Type::Identifier)) {
            auto next = peekNext();
            if (next && next->type == Token::Type::Identifier) {
                initializer = TRY(variableDeclaration());
            } else {
                initializer = TRY(assignStmt());
            }
        }
    }
    
    TRY(eat(Token::Type::Semicolon));
    
    if (!check(Token::Type::Semicolon)) {
        condition = TRY(expression());
    }
    
    TRY(eat(Token::Type::Semicolon));
    
    if (!check(Token::Type::RightParenthesis)) {
        if (check(Token::Type::Identifier)) {
            std::string varName = currentToken.text;
            TRY(eat(Token::Type::Identifier));
            
            if (match({Token::Type::Plus, Token::Type::Plus})) {
                auto right = std::make_unique<ast::BinaryExpr>(
                    std::make_unique<ast::Identifier>(varName),
                    ast::Op::Plus,
                    std::make_unique<ast::LiteralNumber>(1.0)
                );
                increment = std::make_unique<ast::AssignStmt>(varName, std::move(right));
            }
            else if (match({Token::Type::Equal})) {
                auto expr = TRY(expression());
                increment = std::make_unique<ast::AssignStmt>(varName, std::move(expr));
            }
            else {
                return ErrorMeta{"Expected '++' or '=' in for loop increment, but got '" + currentToken.text + "'", currentToken};
            }
        }
    }
    
    TRY(eat(Token::Type::RightParenthesis));
    
    ast::Statement body;
    if (check(Token::Type::LeftBrace)) {
        body = TRY(block());
    }
    else {
        body = TRY(statement());
    }
    
    return make_ok<ast::ForStmt>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}

ParseResult<ast::Statement> Parser::returnStmt() {
    ast::Expression expr = TRY(expression());
    TRY(eat(Token::Type::Semicolon));

    return make_ok<ast::ReturnStmt>(std::move(expr));
}

ParseResult<ast::Statement> Parser::echoStmt() {
    ast::Expression expr = TRY(expression());
    TRY(eat(Token::Type::Semicolon));

    return make_ok<ast::EchoStmt>(std::move(expr));
}

bool Parser::check(Token::Type type) const {
    if (currentToken.type == Token::Type::EndToken) 
        return false;
    
    return currentToken.type == type;
}

std::optional<Token> Parser::skipUntil(Token::Type type) {
    while (currentToken.type != Token::Type::EndToken) {
        if (currentToken.type == type) {
            return currentToken;
        }
        nextToken();
    }
    return std::nullopt;
}

std::optional<Token> Parser::nextToken() {
    if (tokenIndex + 1 < tokensVector.size()) {
        tokenIndex++;
        currentToken = tokensVector[tokenIndex];
        return currentToken;
    }

    return std::nullopt;
}

std::optional<Token> Parser::peekNext() const {
    if (tokenIndex + 1 < tokensVector.size()) {
        return tokensVector[tokenIndex + 1];
    }

    return std::nullopt;
}

} // namespace parser
