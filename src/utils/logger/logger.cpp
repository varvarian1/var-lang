#include "logger/logger.hpp"

namespace logger {
// Write a timestamped log message to the log file
void Logger::log(LogLevel level, const std::string& message) {
    if (logFile.is_open()) {
        // Format: "YYYY-MM-DD HH:MM:SS[LOG_LEVEL]: message".
        logFile << currentTime() << "[" << stringLogLevel(level) << "]"
                << ": " << message << std::endl;
                
        // Immediate write to disk for crash safety - ensures logs are preserved
        // even if the program crashes immediately after logging
        logFile.flush();
    }
}

// Get current system time formatted as YYYY-MM-DD HH:MM:SS
std::string Logger::currentTime() {
    time_t now = time(0); // Get current system time
    tm* tstruct = localtime(&now); // Convert to local time structure
    char buf[80];

    // Format time according to ISO-like standard
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tstruct);
    return std::string(buf); 
}

// Convert LogLevel enum to its string representation
std::string Logger::stringLogLevel(LogLevel level) {
    switch (level) {
        case INFO:
            return "INFO";
        case ERROR:
            return "ERROR";
        case WARNING:
            return "WARNING";
        case DEBUG:
            return "DEBUG";
        default:
            return "UNKNOWN"; // Fallback for undefined enum values
    }
}

/**
 * Log all tokens from the lexer with their types and text
 * 
 * This function iterates through all tokens and writes them to the log file
 * in a human-readable format showing the token type and its corresponding text.
 */
void logger::Logger::lexerLog(ParsedProgram& program) {
    auto& [ tokens, k_meta ] = program;

    int idx = 0;
    for (const auto& token : tokens) {
        logFile << "Token: ";

        // Log the token type with consistent spacing for alignment
        switch (token.type) {
            case Token::Type::Number:
                logFile << "Number          ";
                break;
            case Token::Type::String:
                logFile << "String          ";
                break;
            case Token::Type::LeftParenthesis:
                logFile << "LeftParenthesis ";
                break;
            case Token::Type::RightParenthesis:
                logFile << "RightParenthesis";
                break;
            case Token::Type::LeftBrace:
                logFile << "LeftBrace       ";
                break;
            case Token::Type::RightBrace:
                logFile << "RightBrace      ";
                break;
            case Token::Type::Semicolon:
                logFile << "Semicolon       ";
                break;
            case Token::Type::Comma:
                logFile << "Comma           ";
                break;
            case Token::Type::Colon:
                logFile << "Colon           ";
                break;
            case Token::Type::Plus:
                logFile << "Plus            ";
                break;
            case Token::Type::PlusEqual:
                logFile << "PlusEqual       ";
                break;
            case Token::Type::Increment:
                logFile << "Increment       ";
                break;
            case Token::Type::Minus:
                logFile << "Minus           ";
                break;
            case Token::Type::MinusEqual:
                logFile << "MinusEqual      ";
                break;
            case Token::Type::Decrement:
                logFile << "Decrement       ";
                break;
            case Token::Type::Mult:
                logFile << "Mult            ";
                break;
            case Token::Type::Div:
                logFile << "Div             ";
                break;
            case Token::Type::Equal:
                logFile << "Equal           ";
                break;
            case Token::Type::Less:
                logFile << "Less            ";
                break;
            case Token::Type::LessEqual:
                logFile << "LessEqual       ";
                break;
            case Token::Type::Greater:
                logFile << "Greater         ";
                break;
            case Token::Type::GreaterEqual:
                logFile << "GreaterEqual    ";
                break;
            case Token::Type::NotEqual:
                logFile << "NotEqual        ";
                break;
            case Token::Type::Identifier:
                logFile << "Identifier      ";
                break;

            // ----- Keywords -----
            case Token::Type::Keyword: {
                if (auto* keyword = k_meta.get(idx)) {
                    switch (*keyword) {
                        case Token::Keyword::LET:
                            logFile << "let     ";
                        case Token::Keyword::INT:
                            logFile << "int	";
                            break;
                        case Token::Keyword::FLOAT:
                            logFile << "float	";
                            break;
                        case Token::Keyword::STR:
                            logFile << "str  	";
                            break;
                        case Token::Keyword::IF:
                            logFile << "if      ";
                            break;
                        case Token::Keyword::ELSE:
                            logFile << "else    ";
                            break;
                        case Token::Keyword::FOR:
                            logFile << "for     ";
                            break;
                        case Token::Keyword::AS:
                            logFile << "as	    ";
                            break;
                        case Token::Keyword::FUNCTION:
                            logFile << "function";
                            break;
                        case Token::Keyword::RETURN:
                            logFile << "return  ";
                            break;
                        case Token::Keyword::ECHO:
                            logFile << "echo    ";
                            break;
                    }
                }
            } break;

            case Token::Type::Unknown:
            default:
                logFile << "Unknown         ";
                break;
        }

        // Append the actual token text for reference
        logFile << " -> '" << token.text << "'";
        logFile << std::endl;

        idx++;
    }
}

void Logger::parserLog() {
	
}

} // namespace logger
