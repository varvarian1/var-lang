#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace token {

#define TOKEN_TYPE_LIST \
    X(Unknown)          \
    X(Number)           \
    X(String)           \
    X(Identifier)       \
    X(Keyword)          \
    X(LeftParenthesis)  \
    X(RightParenthesis) \
    X(LeftBrace)        \
    X(RightBrace)       \
    X(Semicolon)        \
    X(Colon)            \
    X(Comma)            \
    X(Not)              \
    X(Equal)            \
    X(Less)             \
    X(Greater)          \
    X(Plus)             \
    X(Minus)            \
    X(Asterisk)         \
    X(Slash)            \
    X(EndToken)         \

struct Token {
    enum class Type {
        #define X(name) name,
                TOKEN_TYPE_LIST
        #undef X
    };

    struct Position {
        std::string filename;
        size_t line = 1;
        size_t column = 1;
    } pos;

	Type type = Token::Type::Unknown;
	std::string text = "";

    Token() = default;
    Token(Type type, std::string& text, Position pos) : type(type), text(std::move(text)), pos(pos) {}
};

static std::string token_type_to_string(Token::Type type) {
    switch (type) {
        #define X(name) case Token::Type::name: return #name;
                TOKEN_TYPE_LIST
        #undef X
    }

    return "Unknown";
}

template<typename T>
struct MetaChannel {
    std::vector<std::optional<T>> data;

    std::optional<T> get(size_t token) {
        if (token >= data.size()) return std::nullopt;
        if (!data[token]) return std::nullopt;
        
        return data[token];
    }

    void set(size_t token, T value) {
        if (token >= data.size())
            data.resize(token + 1);

        data[token] = std::move(value);
    }
};

#define KEYWORD_LIST   \
    X(INT)             \
    X(FLOAT)           \
    X(STR)             \
    X(BOOL)            \
    X(IF)              \
    X(ELSE)            \
    X(FOR)             \
    X(AS)              \
    X(FUNCTION)        \
    X(RETURN)

enum class Keyword {
    #define X(name) name,
            KEYWORD_LIST
    #undef X
};

static std::string keyword_to_string(Keyword keyword) {
    switch (keyword) {
        #define X(name) case Keyword::name: return #name;
                KEYWORD_LIST
        #undef X
    }

    return "Unknown";
}

struct ParsedProgram {
    std::vector<Token> tokens;

    MetaChannel<Keyword> keyword;
};

}