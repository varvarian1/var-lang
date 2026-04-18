#include <iostream>
#include <getopt.h>
#include <fstream>

#include "cli/arg_parser.hpp"

namespace cli {
bool CLIAParser::parse(int argc, char **argv) {
    int opt;

    while ((opt = getopt(argc, argv, "ht")) != -1) {
        switch (opt) {
            case 'h':
                help();
                break;
            case 't':
                readLog("../log/lexer.log");
                break;
            case 'a':
                readLog("../log/ast.log");
                break;
            default:
                std::cerr << "Unknown argument. Use -h for help.\n";
                break;
        }
    }

    for (int i = optind; i < argc; i++) {
        args.push_back(argv[i]);
    }

    if (args.size() > 0) {
        return true;
    }

    return false;
}

void CLIAParser::help() {
    std::cout << "Usage: var [OPTION]... [FILE]\n"
              << "A interpreter for the VAR programming language.\n\n"
              << "Options:\n"
              << "  -h, --help          display this help and exit\n"
              << "  -t, --tokens        display tokens\n"
              << "  -a, --ast           display abstract syntax tree\n\n";
}

void CLIAParser::readLog(std::string filename) {
    std::string line;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: can't open '" << filename << "' file." << std::endl;
    }

    while (getline(file, line)) {
        std::cout << line << std::endl;
    }
    file.close();
}

} // namespace cli
