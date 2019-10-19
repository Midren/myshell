#ifndef MYSHELL_SHELL_H
#define MYSHELL_SHELL_H

#include <map>
#include <string>


class Shell {
public:

private:
    std::map<std::string, std::string> variables;
    std::string pwd;
};


#endif //MYSHELL_SHELL_H
