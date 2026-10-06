#pragma once

#include <unistd.h> 
#include <fcntl.h> 
#include <string> 
#include <sstream> 
#include <vector> 

std::string	toString(unsigned long value);
int lastC(const std::string str);
std::string decodeUrlPath(const std::string &path);