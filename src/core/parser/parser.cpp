#include <iomanip>
#include <type_traits>

#include "lexer/token.hpp"
#include "parser/parser.hpp"

namespace parser {
bool Parser::is_terminator(Token::Type t) {
    switch (t) {
        case Token::Type::RightParenthesis:
        case Token::Type::RightBrace:
        case Token::Type::Semicolon:
        case Token::Type::EndToken:
            return true;
        default:
            return false;
    }
}

std::vector<ast::Statement> Parser::parse() {
    std::vector<ast::Statement> statements;

    while (true) {
        auto stmt = statement();
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

ParseResult<ast::Statement> Parser::functionDeclaration() {
    std::string functionName = currentToken.text;
    TRY(eat(Token::Type::Identifier));
  
    ast::Expression params = TRY(parseParenthesizedExpr());
    
    std::string returnType = "void";
    if (match({Keyword::AS})) {
        returnType = TRY(parseType());
    }
    
    ast::Statement body = TRY(block());
    return make_ok<ast::FunctionDecl>(functionName, std::move(params), returnType, std::move(body));
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

ParseResult<ast::Statement> Parser::block() {
    std::vector<ast::Statement> statements;

    if (currentToken.type != Token::Type::LeftBrace) {
        return ErrorMeta{"Expected '{' at beginning of block, but got '" + currentToken.text + "'", currentToken};
    }
    
    TRY(eat(Token::Type::LeftBrace));

    while (true) {
        auto stmt = statement();
        if (stmt.ok()) {
            statements.push_back(std::move(stmt.value()));
        } else {
            auto &[ message, token ] = stmt.error();
            std::cerr << token.pos.filename << ":" << token.pos.line << ":" << token.pos.column << " | " << message << std::endl;

            skipUntil(Token::Type::Semicolon);
            nextToken();
        }

        if (is_terminator(currentToken.type)) {
            break;
        }
    }
    
    if (check(Token::Type::RightBrace)) {
        TRY(eat(Token::Type::RightBrace));
    } else if (currentToken.type != Token::Type::EndToken) {
        return ErrorMeta{"Expected '}' at end of block, but got '" + currentToken.text + "'", currentToken};
    }

    return make_ok<ast::BlockStmt>(std::move(statements));
}

ParseResult<ast::Expression> Parser::expression(int min_prec) {
    ast::Expression left = TRY(unary());

    while (true) {
        ast::BinaryOperator op = ast::BinaryOperator::Unknown;      

        switch (currentToken.type) {
            case Token::Type::Comma:            op = ast::BinaryOperator::Comma; break;
            case Token::Type::Plus:             op = ast::BinaryOperator::Add; break;
            case Token::Type::Minus:            op = ast::BinaryOperator::Subtract; break;
            case Token::Type::Asterisk:         op = ast::BinaryOperator::Multiply; break;
            case Token::Type::Slash:            op = ast::BinaryOperator::Divide; break;
            case Token::Type::Equal:
                if (checkNext(Token::Type::Equal)) {
                    op = ast::BinaryOperator::Equal;
                } else {
                    op = ast::BinaryOperator::Assign; 
                }
                break;
            case Token::Type::Not:
                if (checkNext(Token::Type::Equal)) {
                    op = ast::BinaryOperator::NotEqual;
                } else {
                    return ErrorMeta{"Expected '!=' operator, but got '" + currentToken.text + "'", currentToken};
                }
                break;
            case Token::Type::Less:
                if (checkNext(Token::Type::Equal)) {
                    op = ast::BinaryOperator::LessEqual;
                } else {
                    op = ast::BinaryOperator::Less;
                }
                break;
            case Token::Type::Greater:
                if (checkNext(Token::Type::Equal)) {
                    op = ast::BinaryOperator::GreaterEqual;
                } else {
                    op = ast::BinaryOperator::Greater;
                }
                break;
            default: break;
        }

        if (op == ast::BinaryOperator::Unknown)
            break;

        ast::BinaryOperatorInfo info = ast::binary_operator_info[op];

        if ((int)info.precedence < min_prec) break;

        while (info.symbols--)
            nextToken();

        if (op == ast::BinaryOperator::Comma && is_terminator(currentToken.type))
            break;

        int next_min_prec = info.left_associative ? (int)info.precedence + 1
                                                   : (int)info.precedence;

        ast::Expression right = TRY(expression(next_min_prec));
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}

ParseResult<ast::Expression> Parser::unary() {
    ast::UnaryOperator op = ast::UnaryOperator::Unknown;

    switch (currentToken.type) {
        case Token::Type::Plus:
            if (checkNext(Token::Type::Plus)) {
                op = ast::UnaryOperator::PreIncrement;
            } else {
                op = ast::UnaryOperator::UnaryPlus;
            }
            break;
        case Token::Type::Minus:
            if (checkNext(Token::Type::Minus)) {
                op = ast::UnaryOperator::PreDecrement;
            } else {
                op = ast::UnaryOperator::UnaryMinus;
            }
            break;
        default: break;
    }

    if (op == ast::UnaryOperator::Unknown) {
        ast::Expression expr = TRY(postfix());

        if (match({Token::Type::Plus, Token::Type::Plus}))
            return make_ok<ast::UnaryExpr>(std::move(expr), ast::UnaryOperator::PostIncrement, false);
        else if (match({Token::Type::Minus, Token::Type::Minus}))
            return make_ok<ast::UnaryExpr>(std::move(expr), ast::UnaryOperator::PostDecrement, false);

        return expr;
    } else {
        nextToken();
        return make_ok<ast::UnaryExpr>(TRY(unary()), op, true);
    }
}
 
ParseResult<ast::Expression> Parser::postfix() {
    if (check(Token::Type::Number))
        return TRY(parseNumber());

    else if (check(Token::Type::String))
        return TRY(parseString());

    else if (check(Token::Type::Identifier)) {
        std::string name = currentToken.text;
        nextToken();
        
        if (check(Token::Type::LeftParenthesis)) {
            ast::Expression args = TRY(parseParenthesizedExpr());
            return make_ok<ast::FunctionCall>(name, std::move(args));
        } else {
            return make_ok<ast::Identifier>(name);
        }
    }

    else if (check(Token::Type::LeftParenthesis))
        return TRY(parseParenthesizedExpr());
    
    else if (check(Token::Type::Keyword)) {
        std::optional<Keyword> keyword = keywordMeta.get(tokenIndex);

        if (keyword) {
            switch (*keyword) {
                case Keyword::INT:
                case Keyword::FLOAT:
                case Keyword::BOOL:
                case Keyword::STR:
                    return TRY(variableDeclaration());


                default: break;
            }
        }
    }

    return ErrorMeta{"Invalid postfix: expected number, variable or '(' but got '" + currentToken.text + "'", currentToken};
}

ParseResult<ast::Expression> Parser::variableDeclaration() {
    std::string variableType = TRY(parseType());

    std::string variableName = currentToken.text;
    TRY(eat(Token::Type::Identifier));
    
    return make_ok<ast::VarDeclExpr>(variableType, variableName);
}

ParseResult<ast::Expression> Parser::parseNumber() {
    const std::string text = currentToken.text;
    nextToken();

    if (text.find('.') != std::string::npos) {
        double value = std::stod(text);
        return make_ok<ast::LiteralNumber>(value, ast::LiteralNumber::Type::FLOAT);
    }

    long long value = std::stoll(text);
    return make_ok<ast::LiteralNumber>(value, ast::LiteralNumber::Type::INT);
}

ParseResult<ast::Expression> Parser::parseString() {
    std::string str = currentToken.text;
    nextToken();

    return make_ok<ast::LiteralString>(str);
}

ParseResult<ast::Expression> Parser::parseParenthesizedExpr() {
    TRY(eat(Token::Type::LeftParenthesis));
    ast::Expression expr = nullptr;
    if (!check(Token::Type::RightParenthesis)) {
        expr = TRY(expression());
    }
    TRY(eat(Token::Type::RightParenthesis));
    return expr;
}

ParseResult<ast::Statement> Parser::statement() {
    if (currentToken.type == Token::Type::EndToken) {
        return ErrorMeta{"Unexpected end of input", currentToken};
    }

    if (match({Keyword::FUNCTION})) {
        return TRY(functionDeclaration());
    } else if (match({Keyword::IF})) {
        return TRY(ifStmt());
    } else if (match({Keyword::FOR})) {
        return TRY(forStmt());
    } else if (match({Keyword::RETURN})) {
        return TRY(returnStmt());
    } else if (check(Token::Type::LeftBrace)) {
        return TRY(block());
    }

    return TRY(exprStmt());
}

ParseResult<ast::Statement> Parser::exprStmt() {
    ast::Expression expr = TRY(expression());
    TRY(eat(Token::Type::Semicolon));

    return make_ok<ast::ExpressionStmt>(std::move(expr));
}

ParseResult<ast::Statement> Parser::returnStmt() {
    ast::Expression expr = TRY(expression());
    TRY(eat(Token::Type::Semicolon));

    return make_ok<ast::ReturnStmt>(std::move(expr));
}

ParseResult<ast::Statement> Parser::ifStmt() {
    TRY(eat(Token::Type::LeftParenthesis));
    auto condition = TRY(expression());
    TRY(eat(Token::Type::RightParenthesis));
    
    auto thenBranch = TRY(block());
    ast::Statement elseBranch = nullptr;
    
    if (currentToken.type == Token::Type::Keyword) {
        std::optional<Keyword> keyword = keywordMeta.get(tokenIndex);
        if (keyword && *keyword == Keyword::ELSE) {
            nextToken();
            elseBranch = TRY(block());
        }
    }
    
    return make_ok<ast::IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

ParseResult<ast::Statement> Parser::forStmt() {
    TRY(eat(Token::Type::LeftParenthesis));
    
    ast::Expression initializer = TRY(expression());
    TRY(eat(Token::Type::Semicolon));

    ast::Expression condition = TRY(expression());
    TRY(eat(Token::Type::Semicolon));

    ast::Expression increment = TRY(expression());
    
    TRY(eat(Token::Type::RightParenthesis));
    
    ast::Statement body;
    if (check(Token::Type::LeftBrace)) {
        body = TRY(block());
    } else {
        body = TRY(statement());
    }
    
    return make_ok<ast::ForStmt>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}

bool Parser::check(Token::Type type) const {
    if (currentToken.type == Token::Type::EndToken) 
        return false;
    
    return currentToken.type == type;
}

bool Parser::checkNext(Token::Type type) const {
    auto token = peekNext();
    if (!token)
        return false;
    
    return token->type == type;
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
