#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum TokenType
{
    IDENTIFIER,

    LBRACE,
    RBRACE,
    SEMICOLON,

    END_OF_FILE
};

struct Token
{
    TokenType type;
    std::string value;

    size_t line;
    size_t column;

    Token() : type(IDENTIFIER), value(), line(0), column(0) {}
    Token(TokenType t, const std::string &v, size_t l, size_t c)
        : type(t), value(v), line(l), column(c) {}
};

#endif