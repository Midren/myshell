#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "Command.h"

bool is_with_symbol(const std::string &str, char sym) {
    for (auto &c: str)
        if (c == sym)
            return true;
    return false;
}

std::vector<Token> parse(const std::string &line) {
    std::vector<Token> tokens;
    std::stringstream ss(line.substr(0, line.find(("#"))));
    std::string word, tmp;
    while (ss >> word) {
        if (is_with_symbol(word, '\"')) {
            if (word[word.length() - 1] != '\"') {
                std::getline(ss, tmp, '\"');
                word += ' ' + tmp + '\"';
            }
            tokens.emplace_back(word, TokenType::CmdDoubleQuoteWord);
        } else if (is_with_symbol(word, '\'')) {
            if (word[word.length() - 1] != '\'') {
                std::getline(ss, tmp, '\'');
                word += ' ' + tmp + '\'';
            }
            tokens.emplace_back(word, TokenType::CmdQuoteWord);
        } else if (word == "|") {
            tokens.emplace_back(word, TokenType::Pipe);
        } else if (word == "&") {
            tokens.emplace_back(word, TokenType::BackgroundType);
        } else if (word[0] == '$') {
            if (word[1] == '(') {
                std::getline(ss, tmp, ')');
                word += ' ' + tmp + ')';
                tokens.emplace_back(word, TokenType::InlineCmd);
            } else {
                tokens.emplace_back(word, TokenType::Var);
            }
        } else if (word[0] == '>') {
            tokens.emplace_back(word, TokenType::Redirection);
        } else {
            tokens.emplace_back(word, TokenType::CmdWord);
        }
    }
    std::cout << ss.str() << std::endl;
    return tokens;
}

int main() {
    auto line = std::string(R"(ls -a "/some/path/to/file"/$MYVAR | wc -a & 'file.txt' &)");
//    auto line = std::string(R"(ls "/some/path to/file" | wc -a $(echo 'file.txt') &)");
    auto tokens = parse(line);
    for (auto &token: tokens) {
        std::cout << token.value << " " << token.type << std::endl;
    }
    return 0;
}