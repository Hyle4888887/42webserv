#include "configParser.hpp"

void ConfigParser::error(const Token& token, const std::string& message)
{
	std::ostringstream oss;
	oss << "Line " << token.line
		<< ", column " << token.column
		<< ": " << message;
	throw std::runtime_error(oss.str());
}

void ConfigParser::expect(const std::vector<Token> &tokens, size_t &pos, TokenType expected)
{
	if (tokens[pos].type != expected)
		error(tokens[pos], "Unexpected : '" + tokens[pos].value + "'");
}

void ConfigParser::parse(const std::vector<Token> &tokens)
{
	size_t pos = 0;
	while (tokens[pos].type != END_OF_FILE)
	{
		if (tokens[pos].value == "server")
			_config.servers.push_back(parseServer(tokens, pos));
		else
			error(tokens[pos], "Expected 'server'");
	}
}

ServerConfig ConfigParser::parseServer(const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	ServerConfig server;
	std::vector<std::string> doneOptions;
	expect(tokens, pos, LBRACE);
	pos++;
	while (tokens[pos].type != RBRACE)
	{
		for (auto option : doneOptions)
		{
			if (tokens[pos].value == option)
				error(tokens[pos], "'" + option + "' already present, no duplicate allowed");
		}
		if (tokens[pos].value == "listen")
		{
			parseListen(server, tokens, pos);
			doneOptions.push_back("listen");
		}
		else if (tokens[pos].value == "server_name")
		{
			pos++;
			expect(tokens, pos, IDENTIFIER);
			server.serverName = tokens[pos].value;
			pos++;
			expect(tokens, pos, SEMICOLON);
			pos++;
			doneOptions.push_back("server_name");
		}
		else if (tokens[pos].value == "client_max_body_size")
		{
			pos++;
			if (!isNumber(tokens[pos].value))
				error(tokens[pos], "Number expected in 'client_max_body_size'");
			server.clientMaxBodySize = std::atoi(tokens[pos].value.c_str());
			pos++;
			expect(tokens, pos, SEMICOLON);
			pos++;
			doneOptions.push_back("client_max_body_size");
		}
		else if (tokens[pos].value == "error_page")
		{
			pos++;
			expect(tokens, pos, IDENTIFIER);
			if (!isNumber(tokens[pos].value))
				error(tokens[pos], "Number expected in 'error_page'");
			int error_code = std::atoi(tokens[pos].value.c_str());
			if (server.errorPages.count(error_code))
				error(tokens[pos], "Error code " + std::to_string(error_code) + " already present, no duplicate allowed");
			pos++;
			expect(tokens, pos, IDENTIFIER);
			if (!endsWith(tokens[pos].value, ".html"))
				error(tokens[pos], "Error page " + std::to_string(error_code) + " must be a .html file");
			std::ifstream file(tokens[pos].value);
			if (!file)
				error(tokens[pos], "HTML page for error_page " + std::to_string(error_code) + " failed to open, or not found");
			file.close();
			server.errorPages[error_code] = tokens[pos].value;
			pos++;
			expect(tokens, pos, SEMICOLON);
			pos++;
		}
		else if (tokens[pos].value == "location")
			server.locations.push_back(parseLocation(tokens, pos));
		else
			error(tokens[pos], "Unexpected : '" + tokens[pos].value + "'");
	}
	pos++;
	return server;
}

LocationConfig ConfigParser::parseLocation(const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	LocationConfig location;
	std::vector<std::string> doneOptions;
	expect(tokens, pos, IDENTIFIER);
	location.path = tokens[pos].value;
	pos++;
	expect(tokens, pos, LBRACE);
	pos++;
	while (tokens[pos].type != RBRACE)
	{
		for (auto option : doneOptions)
		{
			if (tokens[pos].value == option)
				error(tokens[pos], "'" + option + "' already present, no duplicate allowed");
		}
		if (tokens[pos].value == "root")
		{
			pos++;
			expect(tokens, pos, IDENTIFIER);
			location.root = tokens[pos].value;
			pos++;
			expect(tokens, pos, SEMICOLON);
			pos++;
			doneOptions.push_back("root");
		}
		else if (tokens[pos].value == "index")
		{
			
		}
		else
			error(tokens[pos], "Unexpected : '" + tokens[pos].value + "'");
	}
}

void ConfigParser::parseListen(ServerConfig& server, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	size_t res = tokens[pos].value.find(':');
	if (res != std::string::npos)
	{
		if (!isIPv4(tokens[pos].value.substr(0, res)))
			error(tokens[pos], "Wrong IP address provided in listen");
		server.host = tokens[pos].value.substr(0, res);
		if (!isValidPort(tokens[pos].value.substr(res + 1)))
			error(tokens[pos], "Port is empty or out of range");
		server.port = std::atoi(tokens[pos].value.substr(res + 1).c_str());
	}
	else
	{
		server.host = "0.0.0.0";
		if (!isValidPort(tokens[pos].value))
			error(tokens[pos], "Port is empty or out of range");
		server.port = std::atoi(tokens[pos].value.c_str());
	}
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

bool ConfigParser::endsWith(const std::string& fullString, const std::string& ending)
{
    if (ending.size() > fullString.size())
        return false;
    return fullString.compare(fullString.size() - ending.size(), ending.size(), ending) == 0;
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
	file.close();
}

ConfigParser::~ConfigParser()
{
}

const Config &ConfigParser::getConfig() const
{
    return this->_config;
}
