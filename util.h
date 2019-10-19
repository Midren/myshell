#include <string>
#include <functional>
#include <vector>

bool is_with_symbol(const std::string &str, char sym) {
    //TODO: Add check for escape symbol (\", \' \=)
    for (auto &c: str)
        if (c == sym)
            return true;
    return false;
}

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
