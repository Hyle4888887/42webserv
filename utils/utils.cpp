#include "utils.hpp"
#include <iostream>

// Converts an unsigned integer into its string representation.
std::string	toString(unsigned long value)
{
	std::ostringstream	oss;
	oss << value;
	return oss.str();
}

// Returns the last character of a string.
int lastC(const std::string str) { return str[str.size() - 1]; }

// Converts a hexadecimal digit to its numeric value.
static int hexValue(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

// Decodes URL-encoded characters like %20 back to their original form.
std::string decodeUrlPath(const std::string &path) {
	std::string decoded;
	for (std::size_t i = 0; i < path.size(); ++i) {
		if (path[i] == '%' && i + 2 < path.size()) {
			int high = hexValue(path[i + 1]);
			int low = hexValue(path[i + 2]);
			if (high >= 0 && low >= 0) {
				decoded += static_cast<char>((high << 4) | low);
				i += 2; continue;
			}
		} decoded += path[i];
	}
	return decoded;
}