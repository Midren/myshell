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
        keypad(stdscr, true);
        printw("%s $ ", pwd.c_str());
        std::string line;
        wchar_t c;
        int x, y, start_x = pwd.size() + 3;
        int max_x = start_x;
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
                    max_x = start_x;
                    break;
                case KEY_LEFT:
                    getsyx(y, x);
                    x > start_x ? move(y, x - 1) : move(y, x);
                    break;
                case KEY_RIGHT:
                    getsyx(y, x);
                    x < max_x ? move(y, x + 1) : move(y, x);;
                    break;
                case KEY_UP:
                    // TODO PREVIOUS COMAND
                    break;
                case KEY_DOWN:
                    // TODO NEXT COMMAND
                    break;
                case KEY_BACKSPACE:
                    getsyx(y, x);
                    if (x > start_x) {
                        line.pop_back();
                        move(y, x - 1);
                        delch();
                        max_x--;
                    }

                    break;
                default:
                    addch(c);
                    max_x++;
                    line += c;
            }
            refresh();
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
        for (auto &cmd:token_commands) {
            for (const auto &token:cmd) {
                if (token.type == TokenType::AddVar) {
                    local_variables[token.value.substr(0, token.value.find('='))] =
                            token.value.substr(token.value.find('=') + 1, token.value.size() - line.find(('=')) - 1);
                    continue;
                }
            }
            commands.emplace_back(cmd);
        }

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
    std::map<std::string, std::string> local_variables;
    std::string pwd;
};


#endif //MYSHELL_SHELL_H
