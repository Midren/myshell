#ifndef MYSHELL_SHELL_H
#define MYSHELL_SHELL_H

#include <map>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <curses.h>

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
        } else if (is_with_symbol(word, '=')) {
            tokens.emplace_back(word, TokenType::AddVar);
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

    return
            tokens;
}


class Shell {
public:
    Shell() {
        initscr();
        noecho();
    };

    ~Shell() {
        endwin();
    }

    void start() {
        printw("%s $ ", pwd.c_str());
        std::string line;
        char c;
        while (true) {
            c = getch();
            switch (c) {
                case EOF:
                    goto exit;
                case '\n':
                    addch('\n');
                    execute(line);
                    printw(line.c_str());
                    line.clear();
                    printw("\n%s $ ", pwd.c_str());
                    break;
                default:
                    addch(c);
                    line += c;
            }
        }
        exit:
        return;
    }

    void execute(std::string line) {
        auto tokens = parse(line);
        std::vector<std::vector<Token>> token_commands = split<Token>(tokens, [](const Token &token) {
            return token.type == TokenType::Pipe;
        });
        std::vector<Command> commands;
        for (auto &cmd:token_commands)
            commands.emplace_back(cmd);

//        std::cout << std::endl;
//        for (auto &cmd:commands) {
//            for (auto &token: cmd.tokens) {
//                std::cout << token.value << " ";
//            }
//            std::cout << std::endl;
//            std::cout << std::boolalpha << cmd.is_background << std::endl;
//        }
    }

private:
    std::map<std::string, std::string> variables;
    std::string pwd;
};


#endif //MYSHELL_SHELL_H
