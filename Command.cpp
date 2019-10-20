#include "Command.h"
#include <unistd.h>
#include <cstdlib>
#include <map>
#include <curses.h>
#include <iostream>

#include "util.h"

std::map<std::string, std::function<int(std::vector<Token>,Shell*)>> Command::internal_functions = {
        {std::string("merrno"),  [](std::vector<Token> params, Shell* shell) { return 0; }},
        {std::string("mpwd"),    [](std::vector<Token> params, Shell* shell) {
            printw("%s\n", shell->pwd.c_str());
            return 0;}},
        {std::string("mcd"),     [](std::vector<Token> params, Shell* shell) { return 0; }},
        {std::string("mexit"),   [](std::vector<Token> params, Shell* shell) {
            if (!params.empty()) {
                for (auto &token: params)
                    if (token.value == "-h" || token.value == "--help") {
                        printw("\nmexit [exit code] [-h|--help]\n\nif called without with exit code, exit with 0");
                        return 0;
                    }
                exit(std::stoi(params.front().value));
            }
            exit(0);
        }},
        {std::string("mecho"),   [](std::vector<Token> params, Shell* shell) {
            for (auto &token: params)
                printw("%s ", token.value.c_str());
            return 0;
        }},
        {std::string("mexport"), [](std::vector<Token> params, Shell* shell) {
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

Command::Command(std::vector<Token> &t) {
    if (!t.empty()) {
        cmd_name = t.front().value;
        params = {t.begin() + 1, t.end()};
        set_background_mode();
        set_redirected_files();
    }
}

void Command::execute(Shell *shell) {
    if (internal_functions.find(cmd_name) != internal_functions.end())
        internal_functions[cmd_name](params, shell);

    //TODO: write fork-exec
    addch('\n');
    for (auto &token: params) {
        printw("_%s_ ", token.value.c_str());
    }
    addch('\n');
}

void Command::set_redirected_files() {
    for (size_t i = 0; i < params.size(); i++) {
        if (params[i].type == TokenType::Redirection) {
            if (is_with_symbol(params[i].value, '&')) {
                if (params[i].value[params[i].value.find('&') + 1] == '1') {
                    output_file = error_file;
                } else {
                    error_file = output_file;
                }
                params.erase(params.begin() + i-- + 1);
            } else {
                switch (params[i].value[0]) {
                    case '1':
                    case '>':
                        output_file = params[i + 1].value;
                        break;
                    case '2':
                        error_file = params[i + 1].value;
                        break;
                    case '<':
                        input_file = params[i + 1].value;
                }
                params.erase(params.begin() + i + 1, params.begin() + i + 3);
                i -= 2;
            }
        }
    }
}

void Command::set_background_mode() {
    if (params.empty())
        return;
    if (params[params.size() - 1].type == TokenType::BackgroundType) {
        is_background = true;
        params.erase(params.end() - 1);
    }
}