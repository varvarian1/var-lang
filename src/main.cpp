#include <iostream>
#include <memory>

#include "lexer/lexer.hpp"
#include "file_reader/file_reader.hpp"
#include "cli/arg_parser.hpp"
#include "parser/parser.hpp"
#include "interpreter/interpreter.hpp"

void runRepl() {
    std::string input;

    interpreter::Interpreter interpreter;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input.empty())
            continue;

        if (input == "exit" || input == "quit")
            break;

        try {
            lexer::Lexer lexer;
            auto program = lexer.tokenize(input);

            parser::Parser parser(program);
            auto ast = parser.parse();

            interpreter.interpret(ast);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char *argv[]) {

    cli::CLIAParser argParser;
    if (!argParser.parse(argc, argv)) {
        std::cerr << "Error: Failed to parse command line arguments" << std::endl;
        return 1;
    }

    if (argParser.needRepl()) {
        runRepl();
        return 0;
    }

    if (argParser.needEval()) {
        std::string code = argParser.getEvalCode();
        
        lexer::Lexer lexer;
        auto program = lexer.tokenize(code);
        
        try {
            parser::Parser parser(program);
            auto ast = parser.parse();
            
            interpreter::Interpreter interpreter;
            interpreter.interpret(ast);
            
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        
        return 0;
    }

    const auto& args = argParser.getArgs();
    if (args.empty()) {
        std::cerr << "Usage: " << argv[0] << " <filename.var>" << std::endl;
        return 1;
    }

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.var>" << std::endl;
        return 1;
    }

    std::string filePath = argv[argc-1];

    std::string input = readFile(filePath);
    if (input.empty()) {
        return 1;
    }

    /* lexer */

    lexer::Lexer lexer;
    auto program = lexer.tokenize(input, filePath);

    /* parser */
    
    try {
        parser::Parser parser(program);
        auto ast = parser.parse();

        if (argParser.needCheck()) {
            std::cout << "Syntax check passed successfully!" << std::endl;
            return 0;
        }
        
        /* interpretation */
        
        interpreter::Interpreter interpreter;
        interpreter.interpret(ast);
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Parser error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
