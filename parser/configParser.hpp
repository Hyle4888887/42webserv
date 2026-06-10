#include <iostream>
#include <string>
#include <map>
#include <vector>

struct Location {
	std::string		path;
	std::string		root;
	std::string		index;
	std::string*	allowedMethods;
	bool			autoIndex;
	std::string		uploadDir;
	std::string		cgiExt;
	std::string		cgiPath;
	int				returnCode;
	std::string		returnUrl;
};

struct ParsedConfig {
	std::string					listen;
	std::string					serverName;
	int							clientMaxBodySize;
	std::map<int, std::string>	errorPages;
	std::vector<Location>		locations;
};

class configParser
{
private:
	ParsedConfig	parsedConfig;
public:
	configParser(/* args */);
	~configParser();
};
