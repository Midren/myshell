#ifndef MYSHELL_SHELL_H
#define MYSHELL_SHELL_H

#include <map>
#include <string>
#include <stack>
#include <vector>
#include "Token.h"
#include <curses.h>

const std::string HISTORY_FILE = ".history";

class Shell {
public:
    Shell();

    explicit Shell(bool mode) : is_ncurses(mode) {};

    ~Shell();

    void start();

    void execute(std::string line);

    template<typename... Args>
    void print(const char *str, Args... args) {
        if (is_ncurses)
            printw(str, args...);
        else
            printf(str, args...);
    }

    void print(const char *str) {
        if (is_ncurses)
            printw(str);
        else
            printf("%s", str);
    }

private:
    friend class Command;

    std::map<std::string, std::string> local_variables;
    std::stack<std::string> history;
    std::string pwd;
    ssize_t error_code = 0;
    bool is_ncurses = true;

    void get_env_vars(char **environ);
};

std::vector<Token> parse(const std::string &line);

#endif //MYSHELL_SHELL_H
