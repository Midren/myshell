#include "Token.h"

#include <utility>

#include "util.h"

Token::Token(std::string data, TokenType t) : value(std::move(data)), type(t) {
    size_t sep;
    switch (type) {
        case CmdQuoteWord:
            sep = value.find('\'');
            value = value.substr(0, sep) + value.substr(sep + 1, value.length() - 2 - sep);
            break;
        case CmdDoubleQuoteWord:
            sep = value.find('\"');
            value = value.substr(0, sep) + value.substr(sep + 1, value.length() - 2 - sep);
            break;
        default:
            break;
    }
}
