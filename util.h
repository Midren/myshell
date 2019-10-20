#ifndef MYSHELL_UTIL_CPP
#define MYSHELL_UTIL_CPP

#include <string>
#include <functional>
#include <vector>
#include <sys/types.h>
#include <dirent.h>

std::string parse_wic(std::string data);

bool is_with_symbol(const std::string &str, char sym);

template<typename T>
std::vector<std::vector<T>> split(std::vector<T> &container, std::function<bool(const T &)> const &func) {
    std::vector<std::vector<T>> ret;
    size_t last = 0;
    for (size_t i = 0; i < container.size(); i++) {
        if (func(container[i])) {
            ret.emplace_back(container.begin() + last, container.begin() + i);
            last = i + 1;
        }
    }
    ret.emplace_back(container.begin() + last, container.end());
    return ret;
}


bool matches(std::string text, std::string pattern);

std::string join(const std::vector<std::string> &array, char separator);

std::string parse_wic(std::string data);


#endif
