#ifndef MYSHELL_COMMAND_H
#define MYSHELL_COMMAND_H

#include <vector>
#include <string>
#include <functional>
#include <curses.h>
#include <zconf.h>

#include "Token.h"
#include "Shell.h"

constexpr size_t BUFFSIZE = 4096;

class Command {
public:
    explicit Command(std::vector<Token> &t);

    Command(const Command &c);

    ~Command();

    void execute(Shell *shell);

private:
    void set_redirected_files(std::vector<Token> &params);

    void set_background_mode(std::vector<Token> &params);

    bool is_background = false;
    int input_file = STDIN_FILENO;
    int output_file = STDOUT_FILENO;
    int error_file = STDERR_FILENO;
    std::string cmd_name;
    int cmd_argc;
    char **cmd_argv;
    static std::map<std::string, std::function<int(int, char **, Shell *)>> internal_functions;
};


#endif //MYSHELL_COMMAND_H
