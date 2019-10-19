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
                //TODO: replace wildcards
                break;
        }
    }

    Token(const Token &t) = default;

    TokenType type;
    std::string value;
};


#endif //MYSHELL_TOKEN_H
