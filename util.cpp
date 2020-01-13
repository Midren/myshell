#include "util.h"
#include <dirent.h>
#include <libgen.h>
#include <cstring>
#include <iostream>

bool is_with_symbol(const std::string &str, char sym) {
    //TODO: Add check for escape symbol (\", \' \=)
    for (size_t i = 0; i < str.size(); i++)
        if (str[i] == sym && (i == 0 || str[i - 1] != '\\'))
            return true;
    return false;
}

bool matches(char *text, char *pattern) {
    int n = strlen(text);
    int m = strlen(pattern);
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

std::vector<Token> replace_wildcards(const std::string &data) {
    char *path, *pattern;
    char *str = strdup(data.c_str());
    if (data.find_last_of('/') != std::string::npos) {
        path = strdup(data.substr(0, data.find_last_of('/') + 1).c_str());
        pattern = strdup(data.substr(data.find_last_of('/') + 1, std::string::npos).c_str());
    } else {
        path = strdup("");
        pattern = strdup(str);
    }
    std::vector<Token> files;
    DIR *dp;
    struct dirent *ep;
    dp = opendir(path);
    if (dp != nullptr) {
        while ((ep = readdir(dp))) {
            if (matches(ep->d_name, pattern)) {
                files.emplace_back(ep->d_name, TokenType::CmdWord);
            }
        }
    }
    free(str);
    free(path);
    free(pattern);
    return files;
}
