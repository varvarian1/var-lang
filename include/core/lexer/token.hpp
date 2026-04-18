#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

struct Token {
    enum class Type {
        Unknown,           

        Number,		        // 1, 1.1
        String,             // "some text"
	    Identifier,	        // x, num
        Keyword,

        LeftParenthesis,    // (
        RightParenthesis,   // )
        LeftBrace,          // {
        RightBrace,         // }
        Semicolon,          // ;
        Colon,              // :
        Comma,              // ,
        Plus,               // +
        Minus,              // -
        Mult,               // *
        Div,                // /

        Equal,              // ==
        PlusEqual,          // +=
        MinusEqual,         // -=
        NotEqual,           // !=
        LessEqual,          // <=
        GreaterEqual,       // >=

        Assign,             // =
        Less,               // <
        Greater,            // >

        Increment,          // ++
        Decrement,          // --

        EndToken           // EOF
    };

    enum class Keyword {
	    LET,		        // let

	    INT,                // int
        FLOAT,              // float
	    STR,		        // str

        IF,                 // if
        ELSE,               // else

        FOR,                // for
	
	    AS,		            // as
        FUNCTION,           // func
        RETURN,             // return

        ECHO,               // echo
    };

	Type type = Token::Type::Unknown;

	std::string text = "";
};

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

struct ParsedProgram {
    std::vector<Token> tokens;
    MetaChannel<Token::Keyword> keyword;
};
