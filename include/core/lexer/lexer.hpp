#pragma once

#include <vector>
#include <unordered_map>

#include "token.hpp"

namespace lexer {
class Lexer {
public:
	
	/* Tokenize the input string. */
	ParsedProgram tokenize(std::string& file, std::string& input);

	// Check if character is a digit (0-9)
	inline bool isDigit(char ch) {
    	return (unsigned char)(ch - '0') <= 9;
	}

	// Check if character can be part of a numeric literal (digit or decimal point)
	inline bool isRealNumeric(char ch) {
    	return isDigit(ch) || ch == '.';
	}

	// Check if character can start an identifier (letter or underscore)
    inline bool isIdentifierStart(char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
    }
    
	// Check if character can be inside an identifier (start char or digit)
    inline bool isIdentifierChar(char ch) {
        return isIdentifierStart(ch) || isDigit(ch);
    }

	// Check if character is whitespace (space, tab, newline, etc.)
    inline bool isWhitespace(char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f';
    }

	// Static map of keyword strings to their corresponding Keyword enum values
	static const std::unordered_map<std::string, Keyword> keywords;
};

} // namespace lexer
