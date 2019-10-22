#include "Shell.h"

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <curses.h>
#include <fstream>

#include <unistd.h>
#include <stack>

#include "Command.h"
#include "util.h"

#ifdef __APPLE__
#undef KEY_BACKSPACE
#define KEY_BACKSPACE 127
#endif


std::vector<Token> parse(const std::string &line);

Shell::Shell() {
    std::ifstream history_file{".history"};
    std::stack<std::string> read_history;
    std::string command;
    while (getline(history_file, command)) {
        history.emplace(command);
    }
    history_file.close();
    initscr();
    noecho();
    char *buffer = new char[PATH_MAX];
    auto cwd = getcwd(buffer, PATH_MAX);
    pwd = cwd;
    free(buffer);
}

Shell::~Shell() {
    std::stack<std::string> history_reversed;
    while (!history.empty()) {
        history_reversed.push(history.top());
        history.pop();
    }
    std::ofstream history_file{".history"};
    while (!history_reversed.empty()) {
        history_file << history_reversed.top() << std::endl;
        history_reversed.pop();
    }
    history_file.close();
    endwin();
}

void Shell::start() {
    keypad(stdscr, true);
    printw("%s $ ", pwd.c_str());
    std::string line;
    wchar_t c;
    int x, y, start_x = pwd.size() + 3;
    int max_x = start_x;
    bool new_command = true;
    std::stack<std::string> previous_commands;
    while (true) {
        c = getch();
        switch (c) {
            case EOF:
                goto exit;
            case '\n':
                getsyx(y, x);
                mvaddch(y, max_x, '\n');
                while (!previous_commands.empty()) {
                    history.push(previous_commands.top());
                    previous_commands.pop();
                }
                if (!line.empty())
                    history.push(line);
                execute(line);
                line.clear();
                std::cout.flush();
                printw("\n%s $ ", pwd.c_str());
                start_x = pwd.size() + 3;
                max_x = start_x;
                new_command = true;
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
                if (!history.empty()) {
                    new_command = false;
                    if (!line.empty())
                        previous_commands.push(line);
                    getsyx(y, x);
                    move(y, start_x);
                    clrtoeol();
                    line = history.top();
                    history.pop();
                    printw("%s", line.c_str());
                    max_x = start_x + line.size();
                }
                break;
            case KEY_DOWN:
                if (!previous_commands.empty()) {
                    if (!line.empty())
                        history.push(line);
                    getsyx(y, x);
                    move(y, start_x);
                    clrtoeol();
                    line = previous_commands.top();
                    previous_commands.pop();
                    printw("%s", line.c_str());
                    max_x = start_x + line.size();
                } else if (line.empty())
                    new_command = true;
                break;
            case KEY_BACKSPACE:
                if (!new_command && !line.empty()) {
                    new_command = true;
                    history.push(line);
                    while (!previous_commands.empty()) {
                        history.push(previous_commands.top());
                        previous_commands.pop();
                    }
                }
                getsyx(y, x);
                if (x > start_x) {
                    line.erase(x - start_x - 1, 1);
                    move(y, start_x);
                    move(y, x - 1);
                    delch();
                    max_x--;
                }
                break;
            case KEY_RESIZE:
                refresh();
                break;
            default:
                if (!new_command && !line.empty()) {
                    new_command = true;
                    history.push(line);
                    while (!previous_commands.empty()) {
                        history.push(previous_commands.top());
                        previous_commands.pop();
                    }
                }
                insch(c);
                getsyx(y, x);
                move(y, x + 1);
                max_x++;
                line.insert(line.begin() + x - start_x, c);
        }
        refresh();
    }
    exit:
    return;
}

void Shell::execute(std::string line) {
    auto tokens = parse(line);

    std::vector<std::vector<Token>> token_commands = split<Token>(tokens, [](const Token &token) {
        return token.type == TokenType::Pipe;
    });

    std::vector<Command> commands;

    for (auto &cmd:token_commands) {
        for (size_t i = 0; i < cmd.size(); i++) {
            auto &token = cmd[i];
//        for (auto &token:cmd) {
            if (token.type == TokenType::AddVar) {
                local_variables[token.value.substr(0, token.value.find('='))] =
                        token.value.substr(token.value.find('=') + 1,
                                           token.value.size() - token.value.find(('=')) - 1);
            } else if (token.type == TokenType::Var) {
                token.value = local_variables[token.value.substr(1, token.value.length() - 1)];
                token.type = TokenType::CmdWord;
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
            } else if (token.type == TokenType::CmdWildCard) {
                auto values = replace_wildcards(token.value);
                cmd.insert(cmd.begin() + i, values.begin(), values.end());
                cmd.erase(cmd.begin() + values.size() + i);
            }
        }
        commands.emplace_back(cmd);
    }

    std::for_each(commands.begin(), commands.end(),
                  std::bind(std::mem_fn(&Command::execute), std::placeholders::_1, this));
}

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
        } else if (word.find_first_of("*?[") != std::string::npos) {
            tokens.emplace_back(word, TokenType::CmdWildCard);
        } else {
            tokens.emplace_back(word, TokenType::CmdWord);
        }
    }

    return tokens;
}

