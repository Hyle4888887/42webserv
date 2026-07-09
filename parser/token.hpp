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
};

#endif