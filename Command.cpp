#include "Command.h"
#include <unistd.h>
#include <cstdlib>
#include <map>
#include <curses.h>
#include <iostream>

#include "util.h"
#include <sys/stat.h>
#include <cstring>

#include <unistd.h>
#include <wait.h>

std::map<std::string, std::function<int(std::vector<Token>, Shell *)>> Command::internal_functions = {
        //TODO: Write params check
        {std::string("merrno"),  [](std::vector<Token> params, Shell *shell) {
            printw("%d", shell->error_code);
            return 0;
        }},
        {std::string("mpwd"),    [](std::vector<Token> params, Shell *shell) {
            printw("%s\n", shell->pwd.c_str());
            return 0;
        }},
        {std::string("mcd"),     [](std::vector<Token> params, Shell *shell) {
            ssize_t result = chdir(params[0].value.c_str());
            if (result != -1) {
                char *buffer = new char[PATH_MAX];
                auto cwd = getcwd(buffer, PATH_MAX);
                shell->pwd = cwd;
            } else {
                shell->error_code = errno;
            }
            return 0;
        }},
        {std::string("mexit"),   [](std::vector<Token> params, Shell *shell) {
            for (auto &token: params)
                if (token.value == "-h" || token.value == "--help") {
                    printw("\nmexit [exit code] [-h|--help]\n\nif called without with exit code, exit with 0");
                    return 0;
                }
            shell->error_code = 0;
            if (!params.empty())
                exit(std::stoi(params.front().value));
            exit(0);
        }
        },
        {std::string("mecho"),   [](std::vector<Token> params, Shell *shell) {
            shell->error_code = 0;
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

    cmd_argv = new char *[params.size() + 2];
    cmd_argv[0] = new char[sizeof(cmd_name.c_str())];
    strcpy(cmd_argv[0], cmd_name.c_str());
    for (size_t i = 1; i < params.size() + 1; i++) {
        cmd_argv[i] = new char[params[i - 1].value.size()];
        strcpy(cmd_argv[i], params[i - 1].value.c_str());
    }
}


Command::~Command() {
    for (size_t i = 0; i < (params.size() + 1); i++)
        delete[] cmd_argv[i];
    delete[] cmd_argv;
}

void Command::execute(Shell *shell) {
    if (internal_functions.find(cmd_name) != internal_functions.end())
        internal_functions[cmd_name](params, shell);

    pid_t pid;
    int status;
    if ((pid = fork()) < 0) {
        std::cerr << "Failed to fork" << std::endl;
        shell->error_code = -1;
    } else if (pid > 0) {
        addstr("parent\n");
        if ((pid = waitpid(pid, &status, 0)) < 0) {
            std::cerr << "waitpid error" << std::endl;
            exit(1);
        }
        shell->error_code = status;
    } else {
        addstr("child\n");
        execvp(cmd_name.c_str(), cmd_argv);
    }
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
