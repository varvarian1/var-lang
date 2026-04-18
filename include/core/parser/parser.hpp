#pragma once

#include <iostream>
#include <memory>

#include "lexer/lexer.hpp"
#include "ast/ast.hpp"

namespace parser {
class Parser {
public:
    Parser(ParsedProgram& program) : program(program), tokensVector(program.tokens), tokenIndex(0) {
        if (!tokensVector.empty()) {
            currentToken = tokensVector[tokenIndex];
        }
    }

    std::vector<std::unique_ptr<ast::Stmt>> parse();

    /* declarations */
    std::unique_ptr<ast::Stmt> declaration();
    std::unique_ptr<ast::Stmt> letDeclaration();
    std::unique_ptr<ast::Stmt> functionDeclaration();
    std::unique_ptr<ast::Stmt> variableDeclaration();
    std::unique_ptr<ast::Stmt> block();
    std::string parseType();

    /* expression parsing methods */
    std::unique_ptr<ast::Expr> expression();
    std::unique_ptr<ast::Expr> term();
    std::unique_ptr<ast::Expr> factor();
    std::unique_ptr<ast::Expr> comparison();

    /* token type checking helpers */
    bool isPlus(const Token& token) const;
    bool isMinus(const Token& token) const;
    bool isComparison(const Token& token) const;

    /* helper methods for factor parsing */
    std::unique_ptr<ast::Expr> parseNumber();
    std::unique_ptr<ast::Expr> parseString();
    std::unique_ptr<ast::Expr> parseParenthesizedExpr();

    /* statment parsing methods */
    std::unique_ptr<ast::Stmt> statement();
    std::unique_ptr<ast::Stmt> ifStmt();
    std::unique_ptr<ast::Stmt> forStmt();
    std::unique_ptr<ast::Stmt> returnStmt();
    std::unique_ptr<ast::Stmt> echoStmt();
    std::unique_ptr<ast::Stmt> assignStmt();

    bool check(Token::Type type) const;
    Token nextToken();
    Token peekNext() const;

    template<typename T>
    Token eat(T type) {
        if constexpr (std::is_same_v<T, Token::Type>) {
            if (currentToken.type != type) {
                std::cerr << "Expected token type: " << static_cast<int>(type) 
                        << ", got: " << static_cast<int>(currentToken.type) 
                        << " text: " << currentToken.text << std::endl;
                throw std::runtime_error("Unexpected token type!");
            }

            Token consumed = currentToken;
            nextToken();

            return consumed;
        }
        else if constexpr (std::is_same_v<T, Token::Keyword>) {
            std::cout << "eat(Keyword): expecting keyword: " << static_cast<int>(type) << std::endl;
            std::cout << "Current token: '" << currentToken.text << "', type: " << static_cast<int>(currentToken.type) << std::endl;
            
            eat(Token::Type::Keyword);
            
            auto& [_, k_meta] = program;
            Token::Keyword* keyword = k_meta.get(tokenIndex - 1);
            
            std::cout << "Found keyword: " << (keyword ? std::to_string(static_cast<int>(*keyword)) : "null") << std::endl;
            
            if (!keyword || *keyword != type) {
                std::cerr << "Expected keyword: " << static_cast<int>(type) 
                        << ", got: " << (keyword ? std::to_string(static_cast<int>(*keyword)) : "null") << std::endl;
                throw std::runtime_error("Unexpected keyword!");
            }
            
            Token consumed = currentToken;
            nextToken();

            return consumed;
        }
        
        throw std::runtime_error("Invalid type for eat()");
    }

    template<typename T>
    bool match(const std::initializer_list<T>& types) {
        T currentType;

        if constexpr (std::is_same_v<T, Token::Type>) {
            currentType = currentToken.type;
        }
        else if constexpr (std::is_same_v<T, Token::Keyword>) {
            auto& [ _, k_meta ] = program;
            Token::Keyword* keyword = k_meta.get(tokenIndex);

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
