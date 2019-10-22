#ifndef MYSHELL_TOKEN_H
#define MYSHELL_TOKEN_H

#include <string>

enum TokenType {
    CmdName,
    CmdWord,
    CmdQuoteWord,
    CmdDoubleQuoteWord,
    CmdWildCard,
    AddVar,
    Var,
    InlineCmd,
    Redirection,
    Pipe,
    BackgroundType
};

struct Token {

    explicit Token(std::string data, TokenType t);

    Token(const Token &t) = default;

    TokenType type;
    std::string value;
};


#endif
