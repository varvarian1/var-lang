#include <iostream>
#include <memory>

#include "lexer/lexer.hpp"
#include "logger/logger.hpp"
#include "file_reader/file_reader.hpp"
#include "cli/arg_parser.hpp"
#include "parser/parser.hpp"
#include "interpreter/interpreter.hpp"

int main(int argc, char *argv[]) {

    std::filesystem::path dir = "log";
    std::filesystem::create_directories(dir);

    logger::Logger logger("log/app.log", logger::WRITE);

    logger.log(logger::INFO, "Program started");

    cli::CLIAParser argParser;
    if (!argParser.parse(argc, argv)) {
        logger.log(logger::ERROR, "Failed to parse command line arguments");
        std::cerr << "Error: Failed to parse command line arguments" << std::endl;
        return 1;
    }
    logger.log(logger::INFO, "Command line arguments parsed successfully");

    if (argc < 2) {
        logger.log(logger::ERROR, "No input file specified.");
        std::cerr << "Usage: " << argv[0] << " <filename.var>" << std::endl;
        return 1;
    }

    std::string filePath = argv[argc-1];
    logger.log(logger::INFO, "Input file: " + filePath);

    std::string input = readFile(filePath);
    if (input.empty()) {
        logger.log(logger::ERROR, "Empty input or file read error");
        return 1;
    }

    /* lexer */
    logger.log(logger::INFO, "Starting lexical analysis");
    logger::Logger lexerLog("log/lexer.log", logger::REWRITE);

    lexer::Lexer lexer;
    auto program = lexer.tokenize(filePath, input);
    
    logger.log(logger::INFO, "Lexical analysis completed");
    lexerLog.lexerLog(program);
    logger.log(logger::INFO, "Lexer tokens written to lexer.log");

    /* parser */
    logger.log(logger::INFO, "Starting syntax analysis");
    
    try {
        parser::Parser parser(program);
        auto ast = parser.parse();
        
        logger.log(logger::INFO, "Syntax analysis completed. AST generated successfully");
        
        /* interpretation */
        logger.log(logger::INFO, "Starting interpretation");
        
        interpreter::Interpreter interpreter;
        interpreter.interpret(ast);
        
        logger.log(logger::INFO, "Interpretation completed successfully");
        
    } catch (const std::runtime_error& e) {
        logger.log(logger::ERROR, std::string("Parser error: ") + e.what());
        std::cerr << "Parser error: " << e.what() << std::endl;
        return 1;
    }

    logger.log(logger::INFO, "Program finished successfully");
    return 0;
}
