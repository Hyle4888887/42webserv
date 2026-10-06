#include "lexer.hpp"

// Skip spaces and comments so tokenization ignores non-significant text.
void Lexer::skipWhitespace()
{
	while (this->_i < this->_content.size())
	{
		if (this->_content[_i] == '#')
		{
			while (this->_i < this->_content.size() && this->_content[_i] != '\n')
				advance();
		}
		else if (isspace(this->_content[_i]))
			advance();
		else
			break ;
	}
}

// Convert the configuration text into a list of tokens.
std::vector<Token> Lexer::tokenize()
{
	char	c;

	std::vector<Token> tokens;
	while (this->_i < this->_content.size())
	{
		skipWhitespace();
		if (this->_i >= this->_content.size())
			break ;
		c = this->_content[_i];
		if (c == '{')
		{
			tokens.push_back(Token(LBRACE, "{", this->_line, this->_column));
			advance();
		}
		else if (c == '}')
		{
			tokens.push_back(Token(RBRACE, "}", this->_line, this->_column));
			advance();
		}
		else if (c == ';')
		{
			tokens.push_back(Token(SEMICOLON, ";", this->_line, this->_column));
			advance();
		}
		else
			tokens.push_back(readWord());
	}
	tokens.push_back(Token(END_OF_FILE, "", this->_line, this->_column));
	return (tokens);
}

// Read the next identifier or keyword from the input stream.
Token Lexer::readWord()
{
	char	c;

	std::string value;
	size_t line = this->_line;
	size_t column = this->_column;
	while (this->_i < this->_content.size())
	{
		c = this->_content[_i];
		if (std::isspace(static_cast<unsigned char>(c)) || c == '{' || c == '}'
			|| c == ';')
			break ;
		value += c;
		advance();
	}
	return Token(IDENTIFIER, value, line, column);
}

// Move the cursor forward and update the current line and column.
void Lexer::advance()
{
	if (this->_content[_i] == '\n')
    {
        this->_line++;
        this->_column = 1;
    }
    else
        this->_column++;
    this->_i++;
}

// Initialize the lexer with the raw configuration content.
Lexer::Lexer(const std::string &content) : _content(content), _i(0), _line(1),
	_column(1)
{
}

// Copy another lexer state.
Lexer::Lexer(const Lexer &other) : _content(other._content), _i(other._i),
	_line(other._line), _column(other._column)
{
}

// Assign one lexer state to another.
Lexer &Lexer::operator=(const Lexer &other)
{
	if (this != &other)
	{
		this->_content = other._content;
		this->_i = other._i;
		this->_line = other._line;
		this->_column = other._column;
	}
	return (*this);
}

// Destroy the lexer instance.
Lexer::~Lexer()
{
}
