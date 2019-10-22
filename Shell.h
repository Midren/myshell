#ifndef MYSHELL_SHELL_H
#define MYSHELL_SHELL_H

#include <map>
#include <string>
#include <stack>

class Shell {
public:
    Shell();;

    ~Shell();

    void start();

    void execute(std::string line);

private:
    friend class Command;
    std::map<std::string, std::string> local_variables;
    std::stack<std::string> history;
    std::string pwd;
    ssize_t error_code = 0;
    void get_env_vars(char **environ);
};


#endif //MYSHELL_SHELL_H
