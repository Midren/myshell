//
// Created by midren on 18.10.19.
//

#ifndef MYSHELL_TOKEN_H
#define MYSHELL_TOKEN_H

#include <string>

enum TokenType {
    CmdName,
    CmdWord,
    CmdQuoteWord,
    CmdDoubleQuoteWord,
    Var,
    InlineCmd,
    Redirection,
    Pipe,
    BackgroundType
};

class Token {
public:
    explicit Token(std::string &data, TokenType t) : value(data), type(t) {}

    void process() {

    }

//private:
    TokenType type;
    std::string value;
};


#endif //MYSHELL_TOKEN_H
