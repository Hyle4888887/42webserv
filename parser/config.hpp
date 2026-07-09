#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include "serverConfig.hpp"

struct Config
{
public:
	std::vector<ServerConfig> servers;
};

#endif