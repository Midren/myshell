#include "Shell.h"

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <stack>
#include <cstring>

#include "Command.h"
#include "util.h"

#ifdef __APPLE__
extern char **environ;
#undef KEY_BACKSPACE
#define KEY_BACKSPACE 127
#endif

Shell::Shell() {
    get_env_vars(environ);

    std::ifstream history_file{HISTORY_FILE};
    if (history_file.is_open()) {
        std::stack<std::string> read_history;
        std::string command;
        while (getline(history_file, command)) {
            history.emplace(command);
        }
        history_file.close();
    }

    FILE *fd = fopen("/dev/tty", "r+");
    scr = newterm(nullptr, fd, fd);
    setvbuf(stdout, nullptr, _IONBF, 0);
    noecho();
    scrollok(stdscr, true);

    char *buffer = new char[PATH_MAX];
    auto cwd = getcwd(buffer, PATH_MAX);
    pwd = cwd;

    char *path_ = getenv("PATH");
    std::string path = path_;
    path += ":" + pwd + "/mycat/";
    setenv("PATH", path.c_str(), 1);
    delete[] buffer;
}

Shell::~Shell() {
    delscreen(scr);
}

void Shell::start() {
    keypad(stdscr, true);
    print("%s $ ", pwd.c_str());

    std::string line;
    wchar_t c;
    int x, y, start_y, start_x = pwd.size() + 3;
    getsyx(start_y, x);
    int max_x = start_x;
    int length_of_the_line = getmaxx(stdscr);
    bool new_command = true;
    std::stack<std::string> previous_commands;
    while (true) {
        c = getch();
        switch (c) {
            case EOF:
                goto exit;
            case '\n':
                move(start_y, start_x);
                clrtobot();
                print("%s\n", line.c_str());
                while (!previous_commands.empty()) {
                    history.push(previous_commands.top());
                    previous_commands.pop();
                }
                if (!line.empty())
                    history.push(line);
                execute(line);
                line.clear();
                std::cout.flush();
                print("\n%s $ ", pwd.c_str());
                start_x = pwd.size() + 3;
                refresh();
                getsyx(start_y, max_x);
                new_command = true;
                break;
            case KEY_LEFT:
                getsyx(y, x);
                x > start_x || y > start_y
                ? x > 0 ? move(y, x - 1) : move(y - 1, length_of_the_line - 1)
                : move(y, x);
                break;
            case KEY_RIGHT:
                getsyx(y, x);
                x + (y - start_y) * length_of_the_line < max_x
                ? (x == length_of_the_line - 1) ? move(y + 1, 0) : move(y, x + 1)
                : move(y, x);;
                break;
            case KEY_UP:
                if (!history.empty()) {
                    new_command = false;
                    if (!line.empty())
                        previous_commands.push(line);
                    move(start_y, start_x);
                    clrtobot();
                    line = history.top();
                    history.pop();
                    print("%s", line.c_str());
                    max_x = start_x + line.size();
                }
                break;
            case KEY_DOWN:
                if (!previous_commands.empty()) {
                    if (!line.empty())
                        history.push(line);
                    move(start_y, start_x);
                    clrtobot();
                    line = previous_commands.top();
                    previous_commands.pop();
                    print("%s", line.c_str());
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
                if (x > start_x || y > start_y) {
                    line.erase((x + (y - start_y) * length_of_the_line - start_x) - 1, 1);
                    move(start_y, start_x);
                    clrtobot();
                    print("%s", line.c_str());
                    if (x == 0)
                        move(y - 1, length_of_the_line - 1);
                    else
                        move(y, x - 1);
                    max_x--;
                }
                break;
            case KEY_RESIZE:
                refresh();
                break;
            default:
                getsyx(y, x);
                if (!new_command && !line.empty()) {
                    new_command = true;
                    history.push(line);
                    while (!previous_commands.empty()) {
                        history.push(previous_commands.top());
                        previous_commands.pop();
                    }
                }
                line.insert(line.begin() + (x + (y - start_y) * length_of_the_line - start_x), c);
                move(start_y, start_x);
                clrtobot();
                print("%s", line.c_str());
                (x == length_of_the_line - 1) ? move(y + 1, 0) : move(y, x + 1);
                ++max_x;
                break;
        }
        refresh();
    }
    exit:
    return;
}

void Shell::execute(std::string line) {
    update_history();
    auto tokens = parse(line);

    std::vector<std::vector<Token>> token_commands = split<Token>(tokens, [](const Token &token) {
        return token.type == TokenType::Pipe;
    });

    std::vector<Command> commands;

    for (auto &cmd:token_commands) {
        for (size_t t = 0; t < cmd.size(); t++) {
            auto &token = cmd[t];
            if (token.type == TokenType::AddVar) {
                size_t ind = token.value.find('=');
                local_variables[token.value.substr(0, ind)] =
                        token.value.substr(ind + 1,
                                           token.value.size() - ind - 1);
                cmd.erase(cmd.begin() + t);
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
                cmd.insert(cmd.begin() + t, values.begin(), values.end());
                cmd.erase(cmd.begin() + values.size() + t);
            } else if (token.type == TokenType::InlineCmd) {
                std::string res;
                FILE *tmp = tmpfile();
                int saved_in = dup(STDIN_FILENO),
                        saved_out = dup(STDOUT_FILENO),
                        tmp_fd = fileno(tmp);

                auto sh = Shell(false);
                sh.execute("mcd " + pwd);
                dup2(tmp_fd, STDOUT_FILENO);

                sh.execute(token.value);

                fflush(tmp);
                rewind(tmp);
                const size_t buffer_size = 4096;
                auto *buf = new char[buffer_size + 1];
                while (fread(buf, sizeof(char), buffer_size, tmp) > 0) {
                    if (ferror(tmp) && !feof(tmp)) {
                        perror("Error happened inside internal command");
                        error_code = 4;
                        delete[] buf;
                        return;
                    } else {
                        res += buf;
                    }
                }
                delete[] buf;

                fclose(tmp);
                dup2(saved_out, STDOUT_FILENO);
                dup2(saved_in, STDIN_FILENO);
                close(saved_in);
                close(saved_out);

                token.value = res;
            }
        }
        if (!cmd.empty()) {
            commands.emplace_back(cmd);
        }
    }
    this->print("before loop");
    int tmp_fd;
    for (size_t i = 0; i < commands.size(); i++) {
        if (i == commands.size() - 1) {
            commands[i].set_IFD(tmp_fd);
        } else {
            FILE *tmp = tmpfile();
            tmp_fd = fileno(tmp);
            commands[i - 1].set_OFD(tmp_fd);
            commands[i].set_IFD(tmp_fd);
        }
    }
    std::for_each(commands.begin(), commands.end(),
                  std::bind(std::mem_fn(&Command::execute), std::placeholders::_1, this));
}

void Shell::get_env_vars(char **environ) {
    std::string env_pair;
    size_t ind;
    for (char **p = environ; *p; p++) {
        env_pair = *p;
        ind = env_pair.find('=');
        local_variables[env_pair.substr(0, ind)] = env_pair.substr(ind + 1, env_pair.size() - ind - 1);
    }
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
                if (word.back() != ')') {
                    std::getline(ss, tmp, ')');
                    word += tmp + ")";
                }
                word = word.substr(2, word.size() - 3);
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

// At least, it works
void Shell::update_history() {
    // FIXME: VERY BAD CODE YOU CAN NOT READ FURTHER !!!
    std::stack<std::string> old_history{history};
    std::stack<std::string> history_reversed;
    while (!old_history.empty()) {
        history_reversed.push(old_history.top());
        old_history.pop();
    }
    std::ofstream history_file{".history"};
    while (!history_reversed.empty()) {
        history_file << history_reversed.top() << std::endl;
        history_reversed.pop();
    }
    history_file.close();
}