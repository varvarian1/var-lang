#include <iostream>

#include "lexer/lexer.hpp"

namespace lexer {
ParsedProgram Lexer::tokenize(std::string& input, std::string file) {
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
    Token::Position position{ file, 1, 1 };
    bool decimalPoint = false;

	auto currentChar = input.begin();
	while (currentChar != input.end()) {
        int id = tokens.size();

		switch (currentState) {
			case State::InNewToken:
				lexem.clear();
                decimalPoint = false;

                if (isWhitespace(*currentChar)) {
                    if (*currentChar == '\n') {
                        position.line++;
                        position.column = 1;
                    }
                    position.column++;
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
                    lexem = '=';
                    ++currentChar;
                    currentToken = {Token::Type::Equal, lexem, position};
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '!') {
                    lexem = "!";
                    ++currentChar;
                    currentToken = {Token::Type::Not, lexem, position};
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '<') {
                    lexem = '<';
                    ++currentChar;
                    currentToken = {Token::Type::Less, lexem, position};
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '>') {
                    lexem = '>';
                    ++currentChar;
                    currentToken = {Token::Type::Greater, lexem, position};
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '+') {
                    lexem = '+';
                    ++currentChar;
                    currentToken = {Token::Type::Plus, lexem, position};
					currentState = State::InCompleteToken;
                } else if (*currentChar == '-') {
                    lexem = '-';
                    ++currentChar;
                    currentToken = {Token::Type::Minus, lexem, position};
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '*') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = {Token::Type::Asterisk, lexem, position};
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '/') {
                    if ((currentChar + 1) != input.end() && *(currentChar + 1) == '/') {
                        // Skip single-line comment: // ... \n
                        currentChar += 2;
                        while (currentChar != input.end() && *currentChar != '\n') {
                            ++currentChar;
                        }
                        continue; // handle \n/whitespace on the next iteration

                    } else if ((currentChar + 1) != input.end() && *(currentChar + 1) == '*') {
                        // Skip multi-line comment
                        currentChar += 2;
                        while (currentChar != input.end() && !(*currentChar == '*' && (currentChar + 1) != input.end() && *(currentChar + 1) == '/')) {
                            if (*currentChar == '\n') {
                                position.line++;
                                position.column = 1;
                            }
                            ++currentChar;
                        }
                        if (currentChar != input.end()) {
                            currentChar += 2; // Skip the closing */
                        }

                        continue; // Start next iteration to handle the next token
                    }

                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = {Token::Type::Slash, lexem, position};
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '(') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::LeftParenthesis, lexem, position };
                    currentState = State::InCompleteToken;
                } else if (*currentChar == ')') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::RightParenthesis, lexem, position };
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '{') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::LeftBrace, lexem, position };
                    currentState = State::InCompleteToken;
				} else if (*currentChar == '}') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::RightBrace, lexem, position };
                    currentState = State::InCompleteToken;
				} else if (*currentChar == ';') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::Semicolon, lexem, position };
                    currentState = State::InCompleteToken;
                } else if (*currentChar == ':') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::Colon, lexem, position };
                    currentState = State::InCompleteToken;
                } else if (*currentChar == ',') {
                    lexem += *currentChar;
                    ++currentChar;
                    currentToken = { Token::Type::Comma, lexem, position };
                    currentState = State::InCompleteToken;
                } else if (*currentChar == '\"') {
                    ++currentChar;
                    currentState = State::InString;
                } else {
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
                    k_meta.set(id, it->second);
                    currentToken = { Token::Type::Keyword, lexem, position };
                } else {
                    currentToken = { Token::Type::Identifier, lexem, position };
                }

				currentState = State::InCompleteToken;
            } break;

			case State::InString:
				if (*currentChar == '\"') {
					++currentChar;
					currentToken = { Token::Type::String, lexem, position };
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

                    currentToken = { Token::Type::Number, lexem, position };
                    currentState = State::InCompleteToken;
                    break;
                }

                currentToken = { Token::Type::Number, lexem, position };
                currentState = State::InCompleteToken;
                break;

			case State::InCompleteToken:
                position.column += currentToken.text.size();
                tokens.push_back(currentToken);
                currentState = State::InNewToken;
                break;
		}
	}

    // If the input ended right after recognizing a complete token (e.g. last char is ';'),
    // the loop exits before the InCompleteToken state is flushed into `tokens`.
    if (currentState == State::InCompleteToken) {
        position.column += currentToken.text.size();
        tokens.push_back(currentToken);
        currentState = State::InNewToken;
    }

    currentToken = { Token::Type::EndToken, lexem, position };
    tokens.push_back(currentToken);

	return program;
}

const std::unordered_map<std::string, Keyword> Lexer::keywords = {
    { "int", Keyword::INT },
    { "float", Keyword::FLOAT },
    { "str", Keyword::STR },
    { "bool", Keyword::BOOL},
    { "if", Keyword::IF },
    { "else", Keyword::ELSE },
    { "for", Keyword::FOR },
    { "as", Keyword::AS },
    { "func", Keyword::FUNCTION },
    { "return", Keyword::RETURN },
};

} // namespace lexer
