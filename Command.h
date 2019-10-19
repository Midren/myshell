#ifndef MYSHELL_COMMAND_H
#define MYSHELL_COMMAND_H

#include <vector>

#include "Token.h"

class Command {
public:
    explicit Command(std::vector<Token> &t) : tokens(t) {
        if (tokens[tokens.size() - 1].type == TokenType::BackgroundType) {
            is_background = true;
            tokens.erase(tokens.end() - 1);
        }
    }

//private:
    void set_input_file() {

    }

    void set_output_file() {

    }

    void set_background_mode() {

    }

    bool is_background;
    std::string input_file;
    std::string output_file;
    std::vector<Token> tokens;
};


#endif //MYSHELL_COMMAND_H
