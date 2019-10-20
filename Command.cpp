#include "Command.h"

#include <cstdlib>
#include <map>
#include <curses.h>
#include <unistd.h>

#include "util.h"
#include <sys/stat.h>

struct stat sb; // For cd;

Command::Command(std::vector<Token> &t) : tokens(t) {
    set_background_mode();
    set_redirected_files();
}

void Command::execute(Shell *shell) {
    std::map<std::string, std::function<int(std::vector<Token>)>> internal_functions = {
            {std::string("merrno"),  [](std::vector<Token> params) { return 0; }},
            {std::string("mpwd"),    [](std::vector<Token> params) { return 0; }},
            {std::string("mcd"),     [&shell](std::vector<Token> params) {
                if (stat(params[0].value.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
                    chdir(params[0].value.c_str());
                    shell->pwd = get_current_dir_name();
                } else
                    printw("%s is not a directory!", params[0].value.c_str());
                return 0;
            }},
            {std::string("mexit"),   [&](std::vector<Token> params) {
                for (auto &token: params)
                    if (token.value == "-h" || token.value == "--help") {
                        printw("\nmexit [exit code] [-h|--help]\n\nif called without with exit code, exit with 0");
                        return 0;
                    }
                // TODO HERE NORMAL EXIT WITH DESTRUCTOR CALLS PLEZA
                if (!params.empty())
                    exit(std::stoi(params.front().value));
                exit(0);
            }},
            {std::string("mecho"),   [](std::vector<Token> params) {
                for (auto &token: params)
                    printw("%s ", token.value.c_str());
                return 0;
            }},
            {std::string("mexport"), [&shell](std::vector<Token> params) {
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

void Command::set_redirected_files() {
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

void Command::set_background_mode() {
    if (tokens[tokens.size() - 1].type == TokenType::BackgroundType) {
        is_background = true;
        tokens.erase(tokens.end() - 1);
    }
}
