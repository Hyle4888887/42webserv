#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <map>

class LocationConfig
{
public:
	std::string path;
	std::string root;
	std::string index;
	bool autoIndex;
	std::vector<std::string> allowedMethods;
	std::string uploadDir;
	std::string redirect;
	std::map<std::string, std::string> cgi;
};

#endif