#ifndef MYSHELL_SHELL_H
#define MYSHELL_SHELL_H

#include <map>
#include <list>
#include <string>

class Shell {
public:
    Shell();;

    ~Shell();

    void start();

    void execute(std::string line);

private:
    std::map<std::string, std::string> local_variables;
    std::list<std::string> history;
    std::string pwd;
};


#endif //MYSHELL_SHELL_H
