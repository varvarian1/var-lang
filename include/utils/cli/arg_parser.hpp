#pragma once

#include <vector>
#include <string>

namespace cli {
/**
 * CLI Argument Parser - class for parsring command line arguments
 */
class CLIAParser {
public:
    CLIAParser() = default;

    std::vector<std::string> args;
    bool parse(int argc, char **argv);
    void help();
    void readLog(std::string filename);

};

} // namespace cli