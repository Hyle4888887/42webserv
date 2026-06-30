#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include "serverConfig.hpp"

class Config
{
public:
	std::vector<ServerConfig> servers;
};

#endif