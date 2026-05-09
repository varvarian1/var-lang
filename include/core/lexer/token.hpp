#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <string_view>

#define TOKEN_TYPE_LIST  \
    X(Unknown)           \
    X(Number)            \
    X(String)            \
    X(Identifier)        \
    X(Keyword)           \
    X(LeftParenthesis)   \
    X(RightParenthesis)  \
    X(LeftBrace)         \
    X(RightBrace)        \
    X(Semicolon)         \
    X(Colon)             \
    X(Comma)             \
    X(Plus)              \
    X(Minus)             \
    X(Mult)              \
    X(Div)               \
    X(Equal)             \
    X(PlusEqual)         \
    X(MinusEqual)        \
    X(NotEqual)          \
    X(LessEqual)         \
    X(GreaterEqual)      \
    X(Assign)            \
    X(Less)              \
    X(Greater)           \
    X(Increment)         \
    X(Decrement)         \
    X(EndToken)


struct Token {
    enum class Type {
        #define X(name) name,
                TOKEN_TYPE_LIST
        #undef X
    };

    struct Position {
        std::string_view filename;
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
        default: return "Unknown";
    }
}

template<typename T>
struct MetaChannel {
    std::vector<std::optional<T>> data;

    T* get(size_t token) {
        if (token >= data.size()) return nullptr;
        if (!data[token]) return nullptr;
        
        return &*data[token];
    }

    void set(size_t token, T value) {
        if (token >= data.size())
            data.resize(token + 1);

        data[token] = std::move(value);
    }
};

#define KEYWORD_LIST   \
    X(LET)             \
    X(INT)             \
    X(FLOAT)           \
    X(STR)             \
    X(IF)              \
    X(ELSE)            \
    X(FOR)             \
    X(AS)              \
    X(FUNCTION)        \
    X(RETURN)          \
    X(ECHO)

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
        default: return "Unknown";
    }
}
struct ParsedProgram {
    std::vector<Token> tokens;

    MetaChannel<Keyword> keyword;
};
