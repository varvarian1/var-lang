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

    bool parse(int argc, char **argv);
    void help();

    bool needCheck() const { return check_flag; }
    bool needEval() const { return eval_flag; }
    bool needRepl() const { return repl_flag; }

    const std::string& getEvalCode() const { return eval_code; }
    const std::vector<std::string>& getArgs() const { return args; }

private:
    std::vector<std::string> args;

    bool check_flag = false;
    bool eval_flag = false;
    bool repl_flag = false;

    std::string eval_code;

};

} // namespace cli