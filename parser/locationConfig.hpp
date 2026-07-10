#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <map>

struct LocationConfig
{
	std::string path;
	std::string root;
	std::string index;
	bool autoIndex;
	std::vector<std::string> allowedMethods;
	bool uploadEnabled;
	std::string uploadDir;
	bool hasRedirect;
	int redirectCode;
	std::string redirectURL;
	std::map<std::string, std::string> cgi;
};

#endif