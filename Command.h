#ifndef MYSHELL_COMMAND_H
#define MYSHELL_COMMAND_H

#include <vector>
#include <string>
#include <functional>
#include <curses.h>

#include "Token.h"
#include "Shell.h"

constexpr size_t BUFFSIZE = 4096;

class Command {
public:
    explicit Command(std::vector<Token> &t);

    ~Command();

    void execute(Shell *shell);

private:
    void set_redirected_files(std::vector<Token> &params);

    void set_background_mode(std::vector<Token> &params);

    bool is_background;
    std::string input_file;
    std::string output_file;
    std::string error_file;
    std::string cmd_name;
    int cmd_argc;
    char **cmd_argv;
    static std::map<std::string, std::function<int(int, char **, Shell *)>> internal_functions;
};


#endif //MYSHELL_COMMAND_H
