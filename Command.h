#ifndef MYSHELL_COMMAND_H
#define MYSHELL_COMMAND_H

#include <vector>
#include <string>

#include "Token.h"

class Command {
public:
    explicit Command(std::vector<Token> &t);


    void execute();

private:
    void set_redirected_files();

    void set_background_mode();

    bool is_background;
    std::string input_file;
    std::string output_file;
    std::string error_file;
    std::vector<Token> tokens;
};


#endif //MYSHELL_COMMAND_H
