#pragma once

#include <unistd.h> 
#include <fcntl.h> 
#include <string> 
#include <sstream> 
#include <vector> 
#include "../parser/locationConfig.hpp"
#include "../parser/serverConfig.hpp"


std::string	            toString(unsigned long value);
int                     lastC(const std::string str);
std::string             decodeUrlPath(const std::string &path);
std::string             toLowerCopy(const std::string &s);
std::string             trimCopy(const std::string &s);
std::string             joinPath(const std::string &base, const std::string &suffix);
const LocationConfig    *matchLocation(const std::string &path, const ServerConfig &config);
std::string             resolvePath(const std::string &urlPath, const LocationConfig &location);
std::string             baseName(const std::string &path);
