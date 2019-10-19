#ifndef MYSHELL_TOKEN_H
#define MYSHELL_TOKEN_H

#include <string>

#include <sys/types.h>
#include <dirent.h>

std::string parse_wic(std::string data);

enum TokenType {
    CmdName,
    CmdWord,
    CmdQuoteWord,
    CmdDoubleQuoteWord,
    AddVar,
    Var,
    InlineCmd,
    Redirection,
    Pipe,
    BackgroundType
};

struct Token {
    explicit Token(std::string &data, TokenType t) : value(data), type(t) {
        switch (type) {
            case CmdQuoteWord:
                value = value.substr(1, value.length() - 2);
                break;
            case CmdDoubleQuoteWord:
                value = value.substr(1, value.length() - 2);
                //TODO: replace local variables
                break;
            case Var:
                //TODO: add new variable to shell
                break;
            case InlineCmd:
                //TODO: invoke shell to run command
                break;
            default:
                value = parse_wic(data);
                break;
        }
    }

    Token(const Token &t) = default;

    TokenType type;
    std::string value;
};

bool matches(std::string text, std::string pattern) {
    int n = text.size();
    int m = pattern.size();

    if (m == 0)
        return (n == 0);

    int i = 0, j = 0, textPointer = -1, pattPointer = -1;
    while (i < n) {
        if (text[i] == pattern[j]) {
            i++;
            j++;
        } else if (j < m && pattern[j] == '?') {
            i++;
            j++;
        } else if (j < m && pattern[j] == '*') {
            textPointer = i;
            pattPointer = j;
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

std::string parse_wic(std::string data) {
    size_t path_end = data.find_last_of('/');
    std::string path = data.substr(0, path_end + 1);
    std::string pattern = data.substr(path_end + 1, data.length() - 1);
    std::vector<std::string> files;
    DIR *dp;
    struct dirent *ep;
    dp = opendir(path.c_str());
    if (dp != NULL) {
        while ((ep = readdir(dp))) {
            if (matches(ep->d_name, pattern)) {
                files.emplace_back(ep->d_name);
            }
        }
    }
    return join(files, ' ');
}


#endif //MYSHELL_TOKEN_H
