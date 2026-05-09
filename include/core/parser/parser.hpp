#pragma once

#include <iostream>
#include <memory>

#include "result.hpp"
#include "lexer/lexer.hpp"
#include "ast/ast.hpp"

namespace parser {

struct ErrorMeta {
    std::string message;
    Token token;
};

template<typename T>
using ParseResult = Result<T, ErrorMeta>;

template <typename T, typename... Args>
Result<std::unique_ptr<T>, ErrorMeta> make_ok(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

class Parser {
public:
    Parser(ParsedProgram& program) : program(program), tokensVector(program.tokens), tokenIndex(0) {
        if (!tokensVector.empty()) {
            currentToken = tokensVector[tokenIndex];
        }
    }

    std::vector<ast::Statement> parse();

    /* declarations */
    ParseResult<ast::Statement> declaration();
    ParseResult<ast::Statement> letDeclaration();
    ParseResult<ast::Statement> functionDeclaration();
    ParseResult<ast::Statement> variableDeclaration();
    ParseResult<ast::Statement> block();
    ParseResult<std::string> parseType();

    /* expression parsing methods */
    ParseResult<ast::Expression> expression();
    ParseResult<ast::Expression> term();
    ParseResult<ast::Expression> factor();
    ParseResult<ast::Expression> comparison();

    /* token type checking helpers */
    bool isPlus(const Token& token) const;
    bool isMinus(const Token& token) const;
    bool isComparison(const Token& token) const;

    /* helper methods for factor parsing */
    ParseResult<ast::Expression> parseNumber();
    ParseResult<ast::Expression> parseString();
    ParseResult<ast::Expression> parseParenthesizedExpr();
    ParseResult<ast::Expression> parseFunctionCall(const std::string& name);

    /* statment parsing methods */
    ParseResult<ast::Statement> statement();
    ParseResult<ast::Statement> functionCallStmt();
    ParseResult<ast::Statement> ifStmt();
    ParseResult<ast::Statement> forStmt();
    ParseResult<ast::Statement> returnStmt();
    ParseResult<ast::Statement> echoStmt();
    ParseResult<ast::Statement> assignStmt();

    bool check(Token::Type type) const;
    std::optional<Token> skipUntil(Token::Type type);
    std::optional<Token> nextToken();
    std::optional<Token> peekNext() const;

    template<typename T>
    ParseResult<Token> eat(T type) {
        if constexpr (std::is_same_v<T, Token::Type>) {
            if (currentToken.type != type) {
                return ErrorMeta{"Expected token type: " + token_type_to_string(type) 
                                    + ", got: " + token_type_to_string(currentToken.type)
                                    + " text: " + currentToken.text, currentToken};
            }

            Token consumed = currentToken;
            nextToken();

            return consumed;
        }
        else if constexpr (std::is_same_v<T, Keyword>) {            
            eat(Token::Type::Keyword);
            
            auto& [_, k_meta] = program;
            Keyword* keyword = k_meta.get(tokenIndex - 1);
            
            if (!keyword || *keyword != type) {
                return ErrorMeta{"Expected keyword: " + keyword_to_string(type)
                                    + ", got: " + (keyword ? keyword_to_string(*keyword) : "null"), currentToken};
            }
            
            Token consumed = currentToken;
            nextToken();

            return consumed;
        }
        
        return ErrorMeta{"Unsupported type for eat()", currentToken};
    }

    template<typename T>
    bool match(const std::initializer_list<T>& types) {
        T currentType;

        if constexpr (std::is_same_v<T, Token::Type>) {
            currentType = currentToken.type;
        }
        else if constexpr (std::is_same_v<T, Keyword>) {
            auto& [ _, k_meta ] = program;
            Keyword* keyword = k_meta.get(tokenIndex);

            if (!keyword)
                return false;

            currentType = *keyword;
        }

        for (const auto& type : types) {
            if (currentType == type) {
                nextToken();
                return true;
            }
        }

        return false;
    }

private:
    // Referance to the token vector (externally owned)
    ParsedProgram& program;
    std::vector<Token>& tokensVector;

    // Current parsing position
    size_t tokenIndex = 0;
    Token currentToken;
};

} // namespace parser
