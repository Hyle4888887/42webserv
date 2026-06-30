#include "configParser.hpp"

void ConfigParser::expect(const std::vector<Token> &tokens, size_t &pos, TokenType expected)
{
	if (tokens[pos].type != expected)
		throw std::runtime_error("Unexpected token");
	pos++;
}

void ConfigParser::parse(const std::vector<Token> &tokens)
{
	size_t pos = 0;
	while (tokens[pos].type != END_OF_FILE)
	{
		if (tokens[pos].value == "server")
			_config.servers.push_back(parseServer(tokens, pos));
		else
			throw std::runtime_error("Expected 'server'");
	}
}

ServerConfig ConfigParser::parseServer(const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	ServerConfig server;
	expect(tokens, pos, LBRACE);
	while (tokens[pos].type != RBRACE)
	{
		if (tokens[pos].value == "listen")
			parseListen(server, tokens, pos);
		else if (tokens[pos].value == "server_name")
		{
			pos++;
			if (tokens[pos].type != IDENTIFIER)
				throw std::runtime_error("No server_name provided");
			server.serverName = tokens[pos].value;
			pos++;
			expect(tokens, pos, SEMICOLON);
		}
		else if (tokens[pos].value == "client_max_body_size")
		{
			pos++;
			if (!isNumber(tokens[pos].value))
				throw std::runtime_error("Number expected in client_max_body_size");
			server.clientMaxBodySize = std::atoi(tokens[pos].value.c_str());
			pos++;
			expect(tokens, pos, SEMICOLON);
		}
	}
	pos++;
	return server;
}

LocationConfig ConfigParser::parseLocation(const std::vector<Token> &tokens, size_t &pos)
{
	return LocationConfig();
}

void ConfigParser::parseListen(ServerConfig& server, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	std::cout << tokens[pos].value << std::endl;
	if (tokens[pos].type != IDENTIFIER)
		throw std::runtime_error("Expected a port or/and an IP address at listen");
	size_t res = tokens[pos].value.find(':');
	if (res != std::string::npos)
	{
		if (!isIPv4(tokens[pos].value.substr(0, res)))
			throw std::runtime_error("Wrong IP address provided in listen");
		server.host = tokens[pos].value.substr(0, res);
		if (!isValidPort(tokens[pos].value.substr(res + 1)))
			throw std::runtime_error("Port is empty or out of range");
		server.port = std::atoi(tokens[pos].value.substr(res + 1).c_str());
	}
	else
	{
		server.host = "0.0.0.0";
		if (!isValidPort(tokens[pos].value))
			throw std::runtime_error("Port is empty or out of range");
		server.port = std::atoi(tokens[pos].value.c_str());
	}
	pos++;
	expect(tokens, pos, SEMICOLON);
}

bool ConfigParser::isNumber(const std::string& s)
{
	if (s.empty())
		return false;
	for (size_t i = 0; i < s.size(); i++)
	{
		if (!std::isdigit(static_cast<unsigned char>(s[i])))
			return false;
	}
	return true;
}

bool ConfigParser::isIPv4(const std::string &s)
{
	std::stringstream ss(s);
	std::string token;
	std::vector<std::string> ip;
	while (getline(ss, token, '.'))
	{
		if (!isNumber(token))
			return false;
		int n = std::atoi(token.c_str());
		if (n < 0 || n > 255)
			return false;
		ip.push_back(token);
	}
	if (ip.size() != 4)
		return false;
	return true;
}

bool ConfigParser::isValidPort(const std::string &s)
{
	if (s.empty())
		return false;
	int n = std::atoi(s.c_str());
	if (n < 1 || n > 65535)
		return false;
	return true;
}

ConfigParser::ConfigParser(const std::string &configFile)
{
	std::ifstream file(configFile.c_str());
	if (!file)
		throw std::runtime_error("Config file does not exist, or could not be opened");
	std::stringstream buffer;
	buffer << file.rdbuf();
	Lexer lexer(buffer.str());
	std::vector<Token> tokens = lexer.tokenize();
	parse(tokens);
}

ConfigParser::~ConfigParser()
{
}

