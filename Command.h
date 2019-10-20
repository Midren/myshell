#ifndef MYSHELL_COMMAND_H
#define MYSHELL_COMMAND_H

#include <vector>

#include "Token.h"
#include "util.h"
#include <cstdlib>

class Command {
public:
    explicit Command(std::vector<Token> &t) : tokens(t) {
        set_background_mode();
        set_redirected_files();
    }


    void execute() {
        std::map<std::string, std::function<int(std::vector<Token>)>> internal_functions = {
                {std::string("merrno"),  [](std::vector<Token> params) { return 0; }},
                {std::string("mpwd"),    [](std::vector<Token> params) { return 0; }},
                {std::string("mcd"),     [](std::vector<Token> params) { return 0; }},
                {std::string("mexit"),   [](std::vector<Token> params) {
                    for (auto &token: params)
                        if (token.value == "-h" || token.value == "--help") {
                            printw("\nmexit [exit code] [-h|--help]\n\nif called without with exit code, exit with 0");
                            return 0;
                        }
                    if (!params.empty())
                        exit(std::stoi(params.front().value));
                    exit(0);
                }},
                {std::string("mecho"),   [](std::vector<Token> params) {
                    for (auto &token: params)
                        printw("%s ", token.value.c_str());
                    return 0;
                }},
                {std::string("mexport"), [](std::vector<Token> params) {
                    for (auto &token: params)
                        if (token.value == "-h" || token.value == "--help") {
                            printw("\nmexport [VAR=VAL] [-h|--help]\n\nSets global environmental variable.");
                            return 0;
                        }
                    for (auto &token: params)
                        if (token.type == TokenType::AddVar) {
                            setenv(token.value.substr(0, token.value.find('=')).c_str(),
                                   token.value.substr(token.value.find('=') + 1,
                                                      token.value.size() - token.value.find(('=')) - 1).c_str(), 1);
                            // TODO add to local shell variables
//                            local_variables[token.value.substr(0, token.value.find('='))] =
//                                    token.value.substr(token.value.find('=') + 1, token.value.size() - token.value.find(('=')) - 1);
                        }
                    return 0;

                }}
        };

        auto cmd_name = tokens.front().value;
        tokens.erase(tokens.begin());
        if (internal_functions.find(cmd_name) != internal_functions.end())
            internal_functions[cmd_name](tokens);

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
