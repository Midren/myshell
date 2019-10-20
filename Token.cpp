#include "Token.h"

#include "util.h"

Token::Token(std::string &data, TokenType t) : value(data), type(t) {
    switch (type) {
        case CmdQuoteWord:
            value = value.substr(1, value.length() - 2);
            break;
        case CmdDoubleQuoteWord:
            value = value.substr(1, value.length() - 2);
            break;
        case Var:
        case InlineCmd:
            //TODO: invoke shell to run command
            break;
        default:
            if (data.find_first_of("*?[") != std::string::npos)
                value = parse_wic(data);
            break;
    }
}
