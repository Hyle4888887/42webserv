#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <unistd.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include "lexer.hpp"
#include "config.hpp"
#include "serverConfig.hpp"
#include "locationConfig.hpp"

class ConfigParser
{
private:
	typedef void (ConfigParser::*LocationParser)(
		LocationConfig&,
		const std::vector<Token>&,
		size_t&
	);
	typedef void (ConfigParser::*ServerParser)(
		ServerConfig&,
		const std::vector<Token>&,
		size_t&
	);
	
	std::map<std::string, LocationParser> _locationParsers;
	std::map<std::string, ServerParser> _serverParsers;
	
	Config _config;
	
	void error(const Token& token, const std::string& message);
	void expect(const std::vector<Token>& tokens, size_t& pos, TokenType expected);
	void parse(const std::vector<Token>& tokens);
	
	ServerConfig parseServer(const std::vector<Token>& tokens, size_t& pos);
	void serverInit(ServerConfig& server);
	void parseListen(ServerConfig& server, const std::vector<Token>& tokens, size_t& pos);
	void parseServerName(ServerConfig& server, const std::vector<Token>& tokens, size_t& pos);
	void parseClientMaxBodySize(ServerConfig& server, const std::vector<Token>& tokens, size_t& pos);
	void parseErrorPage(ServerConfig& server, const std::vector<Token>& tokens, size_t& pos);

	LocationConfig parseLocation(const std::vector<Token>& tokens, size_t& pos);
	void locationInit(LocationConfig& location);
	void parseRoot(LocationConfig& location, const std::vector<Token>& tokens, size_t& pos);
	void parseIndex(LocationConfig& location, const std::vector<Token>& tokens, size_t& pos);
	void parseAllowedMethods(LocationConfig& location, const std::vector<Token>& tokens, size_t& pos);
	void parseAutoIndex(LocationConfig& location, const std::vector<Token>& tokens, size_t& pos);
	void parseUploadDir(LocationConfig& location, const std::vector<Token>& tokens, size_t& pos);
	void parseCgi(LocationConfig& location, const std::vector<Token>& tokens, size_t& pos);
	void parseReturn(LocationConfig& location, const std::vector<Token>& tokens, size_t& pos);

	bool endsWith(const std::string& fullString, const std::string& ending);
	bool isNumber(const std::string& s);
	bool isIPv4(const std::string& ip);
	bool isValidPort(const std::string& s);
	bool isDirectory(const std::string& path);

public:
	ConfigParser(const std::string& configFile);
	~ConfigParser();

	const Config& getConfig() const;
};

#endif