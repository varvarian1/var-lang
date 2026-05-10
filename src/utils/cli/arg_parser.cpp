#include <iostream>
#include <getopt.h>
#include <fstream>
#include <cstdlib>

#include "cli/arg_parser.hpp"

namespace cli {
bool CLIAParser::parse(int argc, char **argv) {

    check_flag = false;
    eval_flag = false;
    repl_flag = false;

    static struct option longOptions[] = {
        {"help", no_argument, 0, 'h'}, 
        {"check", no_argument, 0, 'c'},
        {"eval", required_argument, 0, 'e'},
        {0, 0, 0, 0}    
    };

    int opt;
    int optIndex = 0;
    
    opterr = 0;

    while ((opt = getopt_long(argc, argv, ":hce:", longOptions, &optIndex)) != -1) {
        switch (opt) {
            case 'h':
                help();
                exit(0);
                break;
            case 'c':
                check_flag = true;
                break;
            case 'e':
                eval_flag = true;
                if (optarg && optarg[0] != '-') {
                    eval_code = optarg;
                } 
                else if (optind < argc && argv[optind][0] != '-') {
                    eval_code = argv[optind];
                    optind++;
                } 
                else {
                    repl_flag = true;
                }
                break;
            case ':':
                if (optopt == 'e') {
                    eval_flag = true;
                    repl_flag = true;
                }
                break;
            case '?':
            default:
                if (optopt == 'e') {
                    eval_flag = true;
                    repl_flag = true;
                } else {
                    std::cerr << "Unknown argument. Use -h for help.\n";
                }
                break;
        }
    }

    if (check_flag) {
        std::cout << "Running syntax check..." << std::endl;
    }

    for (int i = optind; i < argc; i++) {
        args.push_back(argv[i]);
    }

    return true;
}

void CLIAParser::help() {
    std::cout << "Usage: var [OPTION]... [FILE]\n"
              << "A interpreter for the VAR programming language.\n\n"
              << "Options:\n"
              << "  -h, --help          display this help and exit\n"
              << "  -c, --check         check syntax without executing\n"
              << "  -e  --eval [CODE]   execute CODE or enter REPL mode if no CODE\n\n";
}

} // namespace cli
