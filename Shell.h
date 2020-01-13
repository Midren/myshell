#ifndef MYSHELL_SHELL_H
#define MYSHELL_SHELL_H

#include <map>
#include <string>
#include <stack>
#include <vector>
#include "Token.h"
#include <curses.h>
#include <algorithm>
#include <signal.h>
#include <iostream>

const std::string HISTORY_FILE = ".history";

class Shell {
public:
    Shell();

    explicit Shell(bool mode) : is_ncurses(mode) {};

    ~Shell();

    void start();

    void execute(const std::string &line);

    template<typename... Args>
    void print(const char *str, Args... args) {
        if (is_ncurses) {
            printw(str, args...);
        } else {
            printf(str, args...);
        }
    }

    void print(const char *str) {
        if (is_ncurses)
            printw(str);
        else
            printf("%s", str);
    }

    static void kill_children() {
        std::for_each(Shell::pids.begin(), Shell::pids.end(), [](int pid) {
            kill(pid, SIGKILL);
        });
    }

    static std::vector<int> pids;
private:
    friend class Command;

    void update_history();

    std::map<std::string, std::string> local_variables;
    std::stack<std::string> history;
    std::string pwd;
    ssize_t error_code = 0;
    SCREEN *scr;
    bool is_ncurses = true;

    void get_env_vars(char **environ);

    void replace_vars(std::vector<Token> &cmd);
};

std::vector<Token> parse(const std::string &line);

#endif //MYSHELL_SHELL_H
