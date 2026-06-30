#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include "lexer.hpp"
#include "config.hpp"
#include "serverConfig.hpp"
#include "locationConfig.hpp"

class ConfigParser
{
private:
	Config _config;
	
	void expect(const std::vector<Token>& tokens, size_t& pos, TokenType expected);
	void parse(const std::vector<Token>& tokens);
	
	ServerConfig parseServer(const std::vector<Token>& tokens, size_t& pos);
	LocationConfig parseLocation(const std::vector<Token>& tokens, size_t& pos);
	void parseListen(ServerConfig& server, const std::vector<Token>& tokens, size_t& pos);

	bool isNumber(const std::string& s);
	bool isIPv4(const std::string& s);
	bool isValidPort(const std::string& s);

public:
	ConfigParser(const std::string& configFile);
	~ConfigParser();
};

#endif