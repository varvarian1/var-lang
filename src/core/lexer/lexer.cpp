#include <iostream>

#include "lexer/lexer.hpp"

namespace lexer {
ParsedProgram Lexer::tokenize(std::string& input) {
    ParsedProgram program;
	auto& [ tokens, k_meta ] = program; // tokens vector + keyword metadata
	
	enum State {
        InNewToken,
		InIdentifier,
        InString,
        InNumber,
        InCompleteToken,
    };

	State currentState = State::InNewToken;
	std::string lexem;
	Token currentToken;
    bool decimalPoint = false;

	auto currentChar = input.begin();
	while (currentChar != input.end()) {
		switch (currentState) {
			case State::InNewToken:
				lexem.clear();
                decimalPoint = false;
				
                if (isWhitespace(*currentChar)) {
		            ++currentChar;
                    continue;
                }

                if (isDigit(*currentChar)) {
                    lexem += *currentChar;
                    ++currentChar;
                    currentState = State::InNumber;
                    break;
                }

                if (isIdentifierStart(*currentChar)) {
                    lexem += *currentChar;
                    ++currentChar;
                    currentState = State::InIdentifier;
                    break;
                }

                else if (*currentChar == '=') {
                    if (currentChar + 1 != input.end() && *(currentChar + 1) == '=') {
                        lexem = "==";
                        currentChar += 2;
                        currentToken = {Token::Type::Equal, lexem};
                        currentState = State::InCompleteToken;
                    }
                    else {
                        lexem = '=';
                        ++currentChar;
                        currentToken = {Token::Type::Equal, lexem};
                        currentState = State::InCompleteToken;
                    }
                }
                else if (*currentChar == '!') {
                    if (currentChar + 1 != input.end() && *(currentChar + 1) == '=') {
                        lexem = "!=";
                        currentChar += 2;
                        currentToken = {Token::Type::NotEqual, lexem};
                        currentState = State::InCompleteToken;
                    }
                }
                else if (*currentChar == '<') {
                    if (currentChar + 1 != input.end() && *(currentChar + 1) == '=') {
                        lexem = "<=";
                        currentChar += 2;
                        currentToken = {Token::Type::LessEqual, lexem};
                        currentState = State::InCompleteToken;
                    }
                    else {
                        lexem = '<';
                        ++currentChar;
                        currentToken = {Token::Type::Less, lexem};
                        currentState = State::InCompleteToken;
                    }
                }
                else if (*currentChar == '>') {
                    if (currentChar + 1 != input.end() && *(currentChar + 1) == '=') {
                        lexem = ">=";
                        currentChar += 2;
                        currentToken = {Token::Type::GreaterEqual, lexem};
                        currentState = State::InCompleteToken;
                    }
                    else {
                        lexem = '>';
                        ++currentChar;
                        currentToken = {Token::Type::Greater, lexem};
                        currentState = State::InCompleteToken;
                    }
                }
                else if (*currentChar == '+') {
					if (currentChar + 1 != input.end() && *(currentChar + 1) == '+') {
						lexem = "++";
						currentChar += 2;
						currentToken = {Token::Type::Increment, lexem};
						currentState = State::InCompleteToken;
					}
                    else if (currentChar + 1 != input.end() && *(currentChar + 1) == '=') {
                        lexem = "+=";
                        currentChar += 2;
                        currentToken = {Token::Type::PlusEqual, lexem};
                        currentState = State::InCompleteToken;
                    }
					else {
						lexem = '+';
						++currentChar;
						currentToken = {Token::Type::Plus, lexem};
						currentState = State::InCompleteToken;
					}
                }
                else if (*currentChar == '-') {
					if (currentChar + 1 != input.end() && *(currentChar + 1) == '-') {
						lexem = "--";
						currentChar += 2;
						currentToken = {Token::Type::Decrement, lexem};
						currentState = State::InCompleteToken;	
					}
                    else if (currentChar + 1 != input.end() && *(currentChar + 1) == '=') {
                        lexem = "-=";
                        currentChar += 2;
                        currentToken = {Token::Type::MinusEqual, lexem};
                        currentState = State::InCompleteToken;
                    }
					else {
						lexem = '-';
						++currentChar;
						currentToken = {Token::Type::Minus, lexem};
						currentState = State::InCompleteToken;
					}
                }
                else if (*currentChar == '*') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = {Token::Type::Mult, lexem};
                    currentState = State::InCompleteToken;
                }
                else if (*currentChar == '/') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = {Token::Type::Div, lexem};
                    currentState = State::InCompleteToken;
                }
                else if (*currentChar == '(') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::LeftParenthesis, lexem };
                    currentState = State::InCompleteToken;
                }
                else if (*currentChar == ')') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::RightParenthesis, lexem };
                    currentState = State::InCompleteToken;
                }
				else if (*currentChar == '{') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::LeftBrace, lexem };
                    currentState = State::InCompleteToken;
				}
				else if (*currentChar == '}') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::RightBrace, lexem };
                    currentState = State::InCompleteToken;
				}
                else if (*currentChar == ';') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::Semicolon, lexem };
                    currentState = State::InCompleteToken;
                }
                else if (*currentChar == ':') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::Colon, lexem };
                    currentState = State::InCompleteToken;
                }
                else if (*currentChar == ',') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::Comma, lexem };
                    currentState = State::InCompleteToken;
                }
                else if (*currentChar == '\"') {
                    ++currentChar;
                    currentState = State::InString;
                }
                else {
                    lexem += *currentChar;
                    ++currentChar;
                    currentState = State::InIdentifier;
                }
				break;

			case State::InIdentifier: {
				while (currentChar != input.end() && isIdentifierChar(*currentChar)) {
					lexem += *currentChar;
					++currentChar;
				}

                auto it = keywords.find(lexem);
                if (it != keywords.end()) {
                    int id = tokens.size();
                    k_meta.set(id, it->second);
                    currentToken = { Token::Type::Keyword, lexem };
                }
                else {
                    currentToken = { Token::Type::Identifier, lexem };
                }

				currentState = State::InCompleteToken;
            } break;

			case State::InString:
				if (*currentChar == '\"') {
					++currentChar;
					currentToken = { Token::Type::String, lexem };
					currentState = State::InCompleteToken;
				}
				else {
					lexem += *currentChar;
					++currentChar;
				}
				break;

            case State::InNumber:
                while (currentChar != input.end() && isRealNumeric(*currentChar)) {
                    if (*currentChar == '.') {
                        if (decimalPoint)
                            std::cerr << "{Lexer} bad numeric construction";
                        else
                            decimalPoint = true;
                    }

                    lexem += *currentChar;
                    ++currentChar;
                }

                if (currentChar != input.end() && isIdentifierStart(*currentChar)) {
                    std::cerr << "{Lexer} Invalid number: " << lexem << *currentChar << std::endl;

                    currentToken = { Token::Type::Number, lexem };
                    currentState = State::InCompleteToken;
                    break;
                }

                currentToken = { Token::Type::Number, lexem };
                currentState = State::InCompleteToken;
                break;

			case State::InCompleteToken:
                tokens.push_back(currentToken);
                currentState = State::InNewToken;
                break;
		}
	}

	return program;
}

const std::unordered_map<std::string, Token::Keyword> Lexer::keywords = {
	{ "let", Token::Keyword::LET },
    { "int", Token::Keyword::INT },
    { "float", Token::Keyword::FLOAT },
    { "str", Token::Keyword::STR },
    { "if", Token::Keyword::IF },
    { "else", Token::Keyword::ELSE },
    { "for", Token::Keyword::FOR },
    { "as", Token::Keyword::AS },
    { "func", Token::Keyword::FUNCTION },
    { "return", Token::Keyword::RETURN },
    { "echo", Token::Keyword::ECHO }
};

} // namespace lexer
