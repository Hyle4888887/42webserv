#ifndef LEXER_HPP
#define LEXER_HPP

#include <iostream>
#include <string>
#include <vector>
#include "token.hpp"

class Lexer
{
private:
	std::string _content;
	size_t		_i;
	size_t		_line;
	size_t		_column;

	void	skipWhitespace();
	Token	readWord();
	void	advance();
public:
	Lexer(const std::string& content);
	Lexer(const Lexer& other);
	Lexer& operator=(const Lexer& other);
	~Lexer();

	std::vector<Token>	tokenize();
};

#endif