#include "Shell.h"
#include <fstream>


int main(int argc, char *argv[]) {
    if (argc > 2)
        return 1;
    if (argc == 2) {
        auto internal_shell = Shell(false);
        std::ifstream script(argv[1]);
        std::string command;
        while (std::getline(script, command))
            internal_shell.execute(command);
        return 0;
    }
    auto shell = Shell();
    shell.start();
    return 0;
}