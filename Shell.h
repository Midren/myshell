#ifndef MYSHELL_SHELL_H
#define MYSHELL_SHELL_H

#include <map>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <curses.h>

#include <unistd.h>

#include "Command.h"
#include "util.h"

std::vector<Token> parse(const std::string &line) {
    std::vector<Token> tokens;
    std::stringstream ss(line.substr(0, line.find('#')));
    std::string word, tmp;
    while (ss >> word) {
        if (is_with_symbol(word, '\"')) {
            if (word[word.length() - 1] != '\"') {
                std::getline(ss, tmp, '\"');
                word += tmp + '\"';
            }
            tokens.emplace_back(word, TokenType::CmdDoubleQuoteWord);
        } else if (is_with_symbol(word, '\'')) {
            if (word[word.length() - 1] != '\'') {
                std::getline(ss, tmp, '\'');
                word += tmp + '\'';
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
        } else if (word[0] == '>' || word[1] == '>' || word[0] == '<') {
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
        char *buffer = new char[PATH_MAX];
        auto cwd = getcwd(buffer, PATH_MAX);
        pwd = std::string(cwd);
        free(buffer);
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
            for (auto &token:cmd) {
                if (token.type == TokenType::AddVar) {
                    local_variables[token.value.substr(0, token.value.find('='))] =
                            token.value.substr(token.value.find('=') + 1, token.value.size() - line.find(('=')) - 1);
                } else if (token.type == TokenType::Var) {
                    token.value = local_variables[token.value.substr(1, token.value.length() - 1)];
                } else if (token.type == TokenType::CmdDoubleQuoteWord) {
                    std::string new_val;
                    size_t last = 0;
                    for (size_t i = 0; i < token.value.size(); i++) {
                        if (token.value[i] == '$') {
                            new_val += token.value.substr(last, i - last);
                            last = i;
                            while (token.value[i] != ' ' && (i != token.value.size())) { i++; }
                            new_val += local_variables[token.value.substr(last + 1, i - last - 1)];
                            last = i;
                        }
                    }
                    if (last != token.value.size())
                        new_val += token.value.substr(last, token.value.size() - last);
                    token.value = new_val;
                    token.type = TokenType::CmdWord;
                }
            }
            commands.emplace_back(cmd);
        }

        std::for_each(commands.begin(), commands.end(), std::mem_fn(&Command::execute));
    }

private:
    std::map<std::string, std::string> local_variables;
    std::string pwd;
};


#endif //MYSHELL_SHELL_H
