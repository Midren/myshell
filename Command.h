#ifndef MYSHELL_COMMAND_H
#define MYSHELL_COMMAND_H

#include <vector>
#include <string>
#include <functional>
#include <curses.h>

#include "Token.h"
#include "Shell.h"

class Command {
public:
    explicit Command(std::vector<Token> &t);


    void execute(Shell *shell);

private:
    void set_redirected_files();

    void set_background_mode();

    bool is_background;
    std::string input_file;
    std::string output_file;
    std::string error_file;
    std::string cmd_name;
    std::vector<Token> params;
    static std::map<std::string, std::function<int(std::vector<Token>,Shell*)>> internal_functions;
};


#endif //MYSHELL_COMMAND_H
