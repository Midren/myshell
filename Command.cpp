#include "Command.h"
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <map>
#include <curses.h>
#include <iostream>

#include "util.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#ifndef __APPLE__

#include <wait.h>

#endif

std::map<std::string, std::function<int(int argc, char **argv, Shell *)>> Command::internal_functions = {
        {std::string("merrno"),  [](int argc, char **argv, Shell *shell) {
            for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-h") != 0 || strcmp(argv[1], "--help") != 0) {
                    shell->print("merrno [-h|--help] -- show error code of last command\n");
                    return 0;
                } else {
                    shell->error_code = 1;
                    return 1;
                }
            }
            shell->error_code = 0;
            shell->print("%d", shell->error_code);
            return 0;
        }},
        {std::string("mpwd"),    [](int argc, char **argv, Shell *shell) {
            for (int i = 1; i < argc; i++) {
                if ((strcmp(argv[i], "-h") != 0 || strcmp(argv[1], "--help") != 0) && argv[i][0] == '-') {
                    shell->print("\nmpwd [-h|--help] -- show current directory\n");
                    return 0;
                } else {
                    shell->error_code = 1;
                }
            }
            shell->error_code = 0;
            shell->print("%s\n", shell->pwd.c_str());
            return 0;
        }},
        {std::string("mcd"),     [](int argc, char **argv, Shell *shell) {
            for (int i = 1; i < argc; i++) {
                if ((strcmp(argv[i], "-h") != 0 || strcmp(argv[1], "--help") != 0) && argv[i][0] == '-') {
                    shell->print("\nmcd <path> [-h|--help]  -- Go to path <path>\n");
                    shell->error_code = 0;
                    return 0;
                } else
                    shell->error_code = 1;
            }
            ssize_t result = chdir(argv[1]);
            if (result != -1) {
                char *buffer = new char[PATH_MAX];
                auto cwd = getcwd(buffer, PATH_MAX);
                shell->pwd = cwd;
            } else {
                shell->error_code = errno;
                if (shell->error_code == EACCES)
                    std::cerr << "Permission is denied for any component of the pathname" << std::endl;
                else if (shell->error_code == ENOENT)
                    std::cerr << "A component of path does not name an existing directory or path is an empty string"
                              << std::endl;
            }
            return 0;
        }},
        {std::string("mexit"),   [](int argc, char **argv, Shell *shell) {
            for (int i = 1; i < argc; i++) {
                if ((strcmp(argv[i], "-h") != 0 || strcmp(argv[1], "--help") != 0) && argv[i][0] == '-') {
                    shell->print("\nmexit [exit code] [-h|--help]\n\nif called without with exit code, exit with 0\n");
                    return 0;
                }
            }
            shell->error_code = 0;
            if (argc > 1)
                exit(std::stoi(argv[1]));
            exit(0);
        }
        },
        {std::string("mecho"),   [](int argc, char **argv, Shell *shell) {
            shell->error_code = 0;
            for (int i = 1; i < argc; i++)
                shell->print("%s ", argv[i]);
            shell->print("\n");
            return 0;
        }
        },
        {std::string("mexport"), [](int argc, char **argv, Shell *shell) {
            for (int i = 1; i < argc; i++) {
                std::string addVarToken(argv[i]);
                if (addVarToken.find('=') != std::string::npos) {
                    size_t ind = addVarToken.find('=');
                    setenv(addVarToken.substr(0, ind).c_str(),
                           addVarToken.substr(ind + 1, addVarToken.size() - ind - 1).c_str(), 1);
                    shell->local_variables[addVarToken.substr(0, ind)] =
                            addVarToken.substr(ind + 1, addVarToken.size() - ind - 1);
                } else {
                    if (shell->local_variables.find(std::string(argv[i])) != shell->local_variables.end())
                        setenv(argv[i], shell->local_variables[std::string(argv[i])].c_str(), 1);
                    else
                        std::cerr << argv[i] << " is not defined!" << std::endl;
                }
            }
            return 0;

        }},
        {std::string("."),       [](int argc, char **argv, Shell *shell) {
            if (argc < 2) {
                std::cerr << std::endl << ".: not enough arguments" << std::endl;
                shell->error_code = 1;
                return 1;
            }
            std::ifstream fin(argv[1]);
            if (!fin.is_open()) {
                std::cerr << std::endl << ".: no such file or directory: " << argv[1] << std::endl;
                shell->error_code = 2;
                return 2;
            }
            std::string line;
            while (std::getline(fin, line)) {
                shell->execute(line);
            }
            return 0;
        }}
};

Command::Command(std::vector<Token> &t) {
    if (!t.empty() && t[0].type != TokenType::AddVar) {
        cmd_name = t.front().value;
        t.erase(t.begin());
        set_background_mode(t);
        set_redirected_files(t);

        cmd_argc = t.size() + 1;
        cmd_argv = new char *[cmd_argc + 1];
        cmd_argv[0] = strdup(cmd_name.c_str());
        for (size_t i = 0; i < t.size(); i++) {
            cmd_argv[i + 1] = strdup(t[i].value.c_str());
        }
        cmd_argv[cmd_argc] = nullptr;
    } else {
        cmd_argc = 0;
    }
}

Command::~Command() {
    for (size_t i = 0; i < cmd_argc; i++)
        free(cmd_argv[i]);
    if (cmd_argc)
        delete[] cmd_argv;
}

void Command::execute(Shell *shell) {
    bool old_ncurses = shell->is_ncurses;
    int saved_in = dup(STDIN_FILENO),
            saved_out = dup(STDOUT_FILENO),
            saved_err = dup(STDERR_FILENO);
    if (input_file != STDIN_FILENO || output_file != STDOUT_FILENO || error_file != STDERR_FILENO)
        shell->is_ncurses = false;

    dup2(input_file, STDIN_FILENO);
    dup2(output_file, STDOUT_FILENO);
    dup2(error_file, STDERR_FILENO);
    if (internal_functions.find(cmd_name) != internal_functions.end())
        internal_functions[cmd_name](cmd_argc, cmd_argv, shell);
    else {
        int child_to_parent[2];
        if (pipe(child_to_parent) == -1) {
            std::cerr << "Error creating pipe" << std::endl;
            shell->error_code = -2;
            return;
        }
        pid_t pid;
        int status;
        if ((pid = fork()) < 0) {
            std::cerr << "Failed to fork" << std::endl;
            shell->error_code = -1;
        } else if (pid > 0) {
            close(child_to_parent[1]);
            if (is_background)
                return;
            if ((pid = waitpid(pid, &status, 0)) < 0) {
                std::cerr << "waitpid error" << std::endl;
                exit(1);
            }

            char buffer[BUFFSIZE];
            FILE *child_input = fdopen(child_to_parent[0], "r");
            do {
                size_t count = fread(buffer, sizeof(char), BUFFSIZE, child_input);
                if (ferror(child_input)) {
                    break;
                }
                if (count >= 0)
                    buffer[count] = '\0';
                shell->print("%s", buffer);
            } while (!feof(child_input));
            fclose(child_input);

            shell->error_code = status;
        } else {
            close(child_to_parent[0]);
            dup2(child_to_parent[1], STDOUT_FILENO);
            execvp(cmd_name.c_str(), cmd_argv);
            std::cerr << "Failed to exec!" << std::endl;
            exit(1);
        }
    }
    fsync(STDIN_FILENO);
    fsync(STDOUT_FILENO);
    fsync(STDERR_FILENO);
    dup2(saved_in, STDIN_FILENO);
    dup2(saved_out, STDOUT_FILENO);
    dup2(saved_err, STDERR_FILENO);
    close(saved_in);
    close(saved_out);
    close(saved_err);
    shell->is_ncurses = old_ncurses;
}

void Command::set_redirected_files(std::vector<Token> &params) {
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
                        output_file = open(params[i + 1].value.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
                        break;
                    case '2':
                        error_file = open(params[i + 1].value.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
                        break;
                    case '<':
                        input_file = open(params[i + 1].value.c_str(), O_RDONLY, 0777);
                }
                params.erase(params.begin() + i, params.begin() + i + 2);
                i -= 1;
            }
        }
    }
}

void Command::set_background_mode(std::vector<Token> &params) {
    if (params.empty())
        return;
    if (params[params.size() - 1].type == TokenType::BackgroundType) {
        is_background = true;
        params.erase(params.end() - 1);
    }
}

Command::Command(const Command &c) : is_background(c.is_background), input_file(c.input_file),
                                     output_file(c.output_file), error_file(c.error_file),
                                     cmd_name(c.cmd_name), cmd_argc(c.cmd_argc) {

    cmd_argv = new char *[cmd_argc + 1];
    cmd_argv[0] = strdup(cmd_name.c_str());
    for (size_t i = 0; i < cmd_argc; i++) {
        cmd_argv[i + 1] = strdup(cmd_argv[i]);
    }
    cmd_argv[cmd_argc] = nullptr;
}

void Command::set_IFD(const int &fd) {
    input_file = fd;
}

void Command::set_OFD(const int &fd) {
    output_file = fd;
}
