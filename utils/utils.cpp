#include "utils.hpp"

// Convert an unsigned integer to a string.
std::string	toString(unsigned long value)
{
	std::ostringstream	oss;
	oss << value;
	return oss.str();
}

// Return the last character of a string.
int lastC(const std::string str) { return str[str.size() - 1]; }