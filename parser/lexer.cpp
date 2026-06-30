#include "lexer.hpp"

void Lexer::skipWhitespace()
{
	while (_i < _content.size())
	{
		if (_content[_i] == '#')
		{
			while (_i < _content.size() && _content[_i] != '\n')
				_i++;
		}
		else if (isspace(_content[_i]))
			_i++;
		else
			break ;
	}
}

std::vector<Token> Lexer::tokenize()
{
	char	c;

	std::vector<Token> tokens;
	while (_i < _content.size())
	{
		skipWhitespace();
		if (_i >= _content.size())
			break ;
		c = _content[_i];
		if (c == '{')
		{
			tokens.push_back({LBRACE, "{"});
			_i++;
		}
		else if (c == '}')
		{
			tokens.push_back({RBRACE, "}"});
			_i++;
		}
		else if (c == ';')
		{
			tokens.push_back({SEMICOLON, ";"});
			_i++;
		}
		else
			tokens.push_back(readWord());
	}
	tokens.push_back({END_OF_FILE, ""});
	return (tokens);
}

Token Lexer::readWord()
{
	std::string value;
	while (_i < _content.size())
	{
		char c = _content[_i];
		if (std::isspace(static_cast<unsigned char>(c)) ||
			c == '{' || c == '}' || c == ';')
			break;
		value += c;
		_i++;
	}
	return {IDENTIFIER, value};
}

Lexer::Lexer(const std::string &content)
{
	this->_content = content;
	this->_i = 0;
}

Lexer::Lexer(const Lexer &other)
{
	this->_content = other._content;
	this->_i = other._i;
}

Lexer &Lexer::operator=(const Lexer &other)
{
	if (this != &other)
	{
		this->_content = other._content;
		this->_i = other._i;
	}
	return (*this);
}

Lexer::~Lexer()
{
}
