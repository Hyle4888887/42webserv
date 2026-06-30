#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "locationConfig.hpp"

class ServerConfig
{
public:
	std::string host;
	int port;
	std::string serverName;
	size_t clientMaxBodySize;
	std::map<int, std::string> errorPages;
	std::vector<LocationConfig> locations;
};

#endif