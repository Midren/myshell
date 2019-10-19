#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "Command.h"
#include "util.h"

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
    return tokens;
}

int main() {
    auto line = std::string(R"(ls "/some/path to/file" | wc -a $(echo 'file.txt') &)");
    auto tokens = parse(line);
    std::vector<std::vector<Token>> token_commands = split<Token>(tokens, [](const Token &token) {
        return token.type == TokenType::Pipe;
    });
    std::vector<Command> commands;
    for (auto &cmd:token_commands)
        commands.emplace_back(cmd);

    for (auto &cmd:commands) {
        for (auto &token: cmd.tokens) {
            std::cout << token.value << " ";
        }
        std::cout << std::endl;
        std::cout << std::boolalpha << cmd.is_background << std::endl;
    }
    return 0;
}