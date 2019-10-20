#include "util.h"
#include <dirent.h>

bool is_with_symbol(const std::string &str, char sym) {
    //TODO: Add check for escape symbol (\", \' \=)
    for (size_t i = 0; i < str.size(); i++)
        if (str[i] == sym && (i == 0 || str[i - 1] != '\\'))
            return true;
    return false;
}

bool matches(std::string text, std::string pattern) {
    int n = text.size();
    int m = pattern.size();
    bool found = false;
    if (m == 0)
        return (n == 0);

    int i = 0, j = 0, textPointer = -1, pattPointer = -1;
    while (i < n) {
        if (text[i] == pattern[j]) {
            i++;
            j++;
        } else if (j < m and pattern[j] == '?') {
            i++;
            j++;
        } else if (j < m and pattern[j] == '*') {
            textPointer = i;
            pattPointer = j;
            j++;
        } else if (j < m && pattern[j] == '[') {
            j++;
            while (pattern[j] != ']') {
                if (pattern[j] == text[i] && !found) {
                    i++;
                    found = true;
                }
                j++;
            }
            if (!found)
                return false;
            found = false;
            j++;
        } else if (pattPointer != -1) {
            j = pattPointer + 1;
            i = textPointer + 1;
            textPointer++;
        } else
            return false;
    }
    while (j < m && pattern[j] == '*') {
        j++;
    }
    return j == m;

}

std::string join(const std::vector<std::string> &array, const char separator) {
    std::string result;
    size_t size = array.size();
    for (size_t i = 0; i < size; i++) {
        if (i != size - 1)
            result += array[i] + separator;
        else
            result += array[i];
    }
    return result;
}

#define PATH_SEPARATOR '/'

void parse_path(std::string &path) {

}

std::string replace_wildcards(std::string data) {
    std::string path, pattern;
    if (data[0] == PATH_SEPARATOR) {
        size_t path_end = data.find_last_of('/');
        path = data.substr(0, path_end + 1);
        pattern = data.substr(path_end + 1, data.length() - 1);
    } else {
        path = "./";
        pattern = data;
    }
    std::vector<std::string> files;
    DIR *dp;
    struct dirent *ep;
    dp = opendir(path.c_str());
    if (dp != nullptr) {
        while ((ep = readdir(dp))) {
            if (matches(ep->d_name, pattern)) {
                files.emplace_back(ep->d_name);
            }
        }
    }
    return join(files, ' ');
}
