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

static int hexValue(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

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
	} return decoded;
}