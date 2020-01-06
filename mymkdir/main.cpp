#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <vector>
#include <algorithm>

int main(int argc, char *argv[]) {
    bool p_flag = false;
    std::vector<std::string> dirs_to_create;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            std::cout << "mymkdir [-h|--help] [-p]  <dirname>" << std::endl;
        } else if (strcmp(argv[i], "-p") == 0)
            p_flag = true;
        else {
            dirs_to_create.emplace_back(argv[i]);
        }
    }
    std::cout << dirs_to_create.size() << std::endl;
    if (p_flag) {
        for (auto &path: dirs_to_create) {
            std::vector<std::string> path_dirs;
            char *c_path = (char*)malloc(sizeof(path.c_str()));
            strcpy(c_path, path.c_str());
            while (*c_path != '.') {
                path_dirs.emplace_back(basename(c_path));
                c_path = dirname(c_path);
            }
            char prev[PATH_MAX];
            getcwd(prev, sizeof(prev));
            std::reverse(path_dirs.begin(), path_dirs.end());
            for(auto &path_dir: path_dirs){
                if(mkdir(path_dir.c_str(), 0777) != 0 && errno != EEXIST){
                    std::cout << "Error occured!" << std::endl;
                }
                chdir(path_dir.c_str());
            }
            chdir(prev);
        }
    } else {
        for (const auto &i: dirs_to_create) {
            if (mkdir(i.c_str(), 0777) != 0) {
                std::cout << "Error occured!" << std::endl;
            }
            return -1;
        }
    }
    return 0;
}