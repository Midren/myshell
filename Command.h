#ifndef MYSHELL_COMMAND_H
#define MYSHELL_COMMAND_H

#include <vector>

#include "Token.h"
#include "util.h"

class Command {
public:
    explicit Command(std::vector<Token> &t) : tokens(t) {
        set_background_mode();
        set_redirected_files();
    }


    void execute() {
        addch('\n');
        for (auto &token: tokens) {
            printw("_%s_ ", token.value.c_str());
        }
        addch('\n');
    }

private:
    void set_redirected_files() {
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i].type == TokenType::Redirection) {
                if (is_with_symbol(tokens[i].value, '&')) {
                    if (tokens[i].value[tokens[i].value.find('&') + 1] == '1') {
                        output_file = error_file;
                    } else {
                        error_file = output_file;
                    }
                    tokens.erase(tokens.begin() + i-- + 1);
                } else {
                    switch (tokens[i].value[0]) {
                        case '1':
                        case '>':
                            output_file = tokens[i + 1].value;
                            break;
                        case '2':
                            error_file = tokens[i + 1].value;
                            break;
                        case '<':
                            input_file = tokens[i + 1].value;
                    }
                    tokens.erase(tokens.begin() + i + 1, tokens.begin() + i + 3);
                    i -= 2;
                }
            }
        }
    }

    void set_background_mode() {
        if (tokens[tokens.size() - 1].type == TokenType::BackgroundType) {
            is_background = true;
            tokens.erase(tokens.end() - 1);
        }
    }

    bool is_background;
    std::string input_file;
    std::string output_file;
    std::string error_file;
    std::vector<Token> tokens;
};


#endif //MYSHELL_COMMAND_H
