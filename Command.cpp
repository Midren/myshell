#include "Command.h"
#include <unistd.h>
#include <cstdlib>
#include <map>
#include <curses.h>
#include <unistd.h>
#include <iostream>

#include "util.h"
#include <sys/stat.h>

struct stat sb; // For cd;
std::map<std::string, std::function<int(std::vector<Token>, Shell *)>> Command::internal_functions = {
        {std::string("merrno"),  [](std::vector<Token> params, Shell *shell) { return 0; }},
        {std::string("mpwd"),    [](std::vector<Token> params, Shell *shell) {
            printw("%s\n", shell->pwd.c_str());
            return 0;
        }},
        {std::string("mcd"),     [](std::vector<Token> params, Shell *shell) {
            if (stat(params[0].value.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
                chdir(params[0].value.c_str());
                shell->pwd = get_current_dir_name();
            } else
                printw("%s is not a directory!", params[0].value.c_str());
            return 0;
        }},
        {std::string("mexit"),   [](std::vector<Token> params, Shell *shell) {
            for (auto &token: params)
                if (token.value == "-h" || token.value == "--help") {
                    printw("\nmexit [exit code] [-h|--help]\n\nif called without with exit code, exit with 0");
                    return 0;
                }
            // TODO HERE NORMAL EXIT WITH DESTRUCTOR CALLS PLEZA
            if (!params.empty())
                exit(std::stoi(params.front().value));
            exit(0);
        }
        },
        {std::string("mecho"),   [](std::vector<Token> params, Shell *shell) {
            for (auto &token: params)
                printw("%s ", token.value.c_str());
            return 0;
        }},
        {std::string("mexport"), [](std::vector<Token> params, Shell *shell) {
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
                    shell->local_variables[token.value.substr(0, token.value.find('='))] =
                            token.value.substr(token.value.find('=') + 1,
                                               token.value.size() - token.value.find(('=')) - 1);
                } else {
                    if (shell->local_variables.find(token.value) != shell->local_variables.end())
                        setenv(token.value.c_str(), shell->local_variables[token.value].c_str(), 1);
                    else
                        printw("%s is not defined!\n", token.value.c_str());
                }
            return 0;

        }}
};

Command::Command(std::vector<Token> &t) {
    if (!t.empty() && t[0].type != TokenType::AddVar) {
        cmd_name = t.front().value;
        params = {t.begin() + 1, t.end()};
        set_background_mode();
        set_redirected_files();
    }
}

void Command::execute(Shell *shell) {
    if (internal_functions.find(cmd_name) != internal_functions.end())
        internal_functions[cmd_name](params, shell);

    printw("\n%s: ", cmd_name.c_str());
    //TODO: write fork-exec
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