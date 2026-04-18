#pragma once

#include <iostream>
#include <fstream>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

namespace logger {

// Define the severity level of log messages
enum LogLevel {
    INFO,    // General information messages
    ERROR,   // Error conditions
    WARNING, // Warning conditions that don't stop execution
    DEBUG    // Debug information for development
};

// Defines the file writing mode for the logger
enum Mode {
    WRITE,  // Append to existing file
    REWRITE // Overwrite the file
};

/**
 * The Logger class that handles writing log messages to a file
 * Provides functionality for logging messages with different severity levels
 */
class Logger {
public:
    Logger(const std::string& filename, Mode mode) {
        if (mode == WRITE)
            logFile.open(filename, std::ios::app); // Open in append mode
        else if (mode == REWRITE)
            logFile.open(filename, std::ios::out); // Open in write mode (overwrite)
        
        if (!logFile.is_open())
            std::cerr << "Error: can't open file." << std::endl;
    }

    // Write a log message with specified severity level
    void log(LogLevel level, const std::string& message);

    // Get current system time as formatted string
    std::string currentTime();

    // Convert LogLevel enum to human-readable string
    std::string stringLogLevel(LogLevel level);

    // Log all tokens from the lexer in a formatted way
    void lexerLog(ParsedProgram& program);

	void parserLog();

    // Destroy the Logger object and close the log file
    ~Logger() {
        logFile.close(); // Ensure file is properly closed
    }

private:
    std::ofstream logFile; // File stream for logging output

};

} // namespace logger
