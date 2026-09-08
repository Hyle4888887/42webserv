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
	serverInit(server);
	std::set<std::string> doneOptions;
	expect(tokens, pos, LBRACE);
	pos++;
	while (tokens[pos].type != RBRACE)
	{
		std::string directive = tokens[pos].value;
		expect(tokens, pos, IDENTIFIER);
		if (directive == "location")
			server.locations.push_back(parseLocation(tokens, pos));
		else
		{
			if (directive != "error_page")
			{
				if (!doneOptions.insert(directive).second)
					error(tokens[pos], "'" + directive + "' already present");
			}
			std::map<std::string, ServerParser>::iterator it = _serverParsers.find(directive);
			if (it == _serverParsers.end())
				error(tokens[pos], "Unknown directive '" + directive + "'");
			(this->*(it->second))(server, tokens, pos);
		}
	}
	pos++;
	return server;
}

void ConfigParser::serverInit(ServerConfig &server)
{
	server.host = "0.0.0.0";
	server.port = 80;
	server.serverName = "";
	server.clientMaxBodySize = 1048576; // 1Mb
}

void ConfigParser::parseListen(ServerConfig& server, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (std::count(tokens[pos].value.begin(), tokens[pos].value.end(), ':') > 1)
    	error(tokens[pos], "Invalid listen format");
	size_t res = tokens[pos].value.find(':');
	std::string left = tokens[pos].value.substr(0, res);
	std::string right = tokens[pos].value.substr(res + 1);
	if (res == std::string::npos)
	{
		if (isValidPort(tokens[pos].value))
			server.port = std::atoi(tokens[pos].value.c_str());
		else if (isIPv4(tokens[pos].value))
			error(tokens[pos], "Missing port after IP address");
		else
			error(tokens[pos], "Expected a port or an IP:port pair");
	}
	else
	{
		if (left.empty())
			error(tokens[pos], "Missing IP address before ':'");
		if (right.empty())
			error(tokens[pos], "Missing port after ':'");
		if (!isIPv4(left))
			error(tokens[pos], "Invalid IP address");
		if (!isValidPort(right))
			error(tokens[pos], "Invalid port");
		server.host = left;
		server.port = std::atoi(right.c_str());
	}
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

void ConfigParser::parseServerName(ServerConfig &server, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	server.serverName = tokens[pos].value;
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

void ConfigParser::parseClientMaxBodySize(ServerConfig &server, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	if (!isNumber(tokens[pos].value))
		error(tokens[pos], "Number expected in 'client_max_body_size'");
	if (tokens[pos].value[0] == '-')
		error(tokens[pos], "'client_max_body_size' must be positive");
	server.clientMaxBodySize = std::atoi(tokens[pos].value.c_str());
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

void ConfigParser::parseErrorPage(ServerConfig &server, const std::vector<Token> &tokens, size_t &pos)
{
	std::ostringstream oss;
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (!isNumber(tokens[pos].value))
		error(tokens[pos], "Number expected in 'error_page'");
	int error_code = std::atoi(tokens[pos].value.c_str());
	if (server.errorPages.count(error_code))
	{
		oss.str("");
		oss << "Error code " << error_code << " already present, no duplicate allowed";
		error(tokens[pos], oss.str());
	}
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (!endsWith(tokens[pos].value, ".html"))
	{
		oss.str("");
		oss << "Error page " << error_code << " must be a .html file";
		error(tokens[pos], oss.str());
	}
	if (access(tokens[pos].value.c_str(), R_OK) != 0)
		error(tokens[pos], "This 'error_page' file doesn't exist or is not readable : " + tokens[pos].value);
	server.errorPages[error_code] = tokens[pos].value;
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

LocationConfig ConfigParser::parseLocation(const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	LocationConfig location;
	locationInit(location);
	std::set<std::string> doneOptions;
	expect(tokens, pos, IDENTIFIER);
	location.path = tokens[pos].value;
	pos++;
	expect(tokens, pos, LBRACE);
	pos++;
	while (tokens[pos].type != RBRACE)
	{
		expect(tokens, pos, IDENTIFIER);
		std::string directive = tokens[pos].value;
		if (directive != "cgi")
		{
			if (!doneOptions.insert(directive).second)
    			error(tokens[pos], "'" + directive + "' already present");
		}
		std::map<std::string, LocationParser>::iterator it = _locationParsers.find(directive);
		if (it == _locationParsers.end())
			error(tokens[pos], "Unknown directive '" + directive + "'");
		(this->*(it->second))(location, tokens, pos);
	}
	pos++;
	return location;
}

void ConfigParser::locationInit(LocationConfig &location)
{
	location.root = "./";
	location.index = "index.html";
	location.allowedMethods.push_back("GET");
	location.autoIndex = false;
	location.uploadEnabled = false;
	location.uploadDir = "";
	location.hasRedirect = false;
	location.path = "/";
}

void ConfigParser::parseRoot(LocationConfig &location, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	location.root = tokens[pos].value;
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

void ConfigParser::parseIndex(LocationConfig &location, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	// if (!endsWith(tokens[pos].value, ".html"))
	// 	error(tokens[pos], "'index' must be a .html file");
	location.index = tokens[pos].value;
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

void ConfigParser::parseAllowedMethods(LocationConfig &location, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	location.allowedMethods.clear();
	while (tokens[pos].type != SEMICOLON)
	{
		expect(tokens, pos, IDENTIFIER);
		for (std::vector<std::string>::const_iterator it = location.allowedMethods.begin(); it != location.allowedMethods.end(); ++it)
		{
			if (*it == tokens[pos].value)
				error(tokens[pos], "'" + *it + "' already present, no duplicate allowed");
		}
		if (tokens[pos].value == "GET" || tokens[pos].value == "POST" || tokens[pos].value == "DELETE")
			location.allowedMethods.push_back(tokens[pos].value);
		else
			error(tokens[pos], "Method '" + tokens[pos].value + "' unknown");
		pos++;
	}
	pos++;
}

void ConfigParser::parseAutoIndex(LocationConfig &location, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (tokens[pos].value == "on")
		location.autoIndex = true;
	else if (tokens[pos].value == "off")
		location.autoIndex = false;
	else
		error(tokens[pos], "Only accepted parameters for 'autoindex' are 'on' or 'off'");
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

void ConfigParser::parseUploadDir(LocationConfig &location, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (tokens[pos].value == "on")
		location.uploadEnabled = true;
	else if (tokens[pos].value == "off")
		location.uploadEnabled = false;
	else
		error(tokens[pos], "Only accepted first parameter for 'upload_dir' are 'on' or 'off'");
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (!isDirectory(tokens[pos].value))
		error(tokens[pos], "Upload directory does not exist or is not a directory");
	location.uploadDir = tokens[pos].value;
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

void ConfigParser::parseCgi(LocationConfig &location, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (tokens[pos].value[0] != '.' || tokens[pos].value == ".")
		error(tokens[pos], "'cgi' first parameter must be an extension");
	if (location.cgi.count(tokens[pos].value))
		error(tokens[pos], "Extension duplicate for 'cgi' : " + tokens[pos].value);
	std::string cgi_extension = tokens[pos].value;
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (access(tokens[pos].value.c_str(), X_OK) != 0)
		error(tokens[pos], "This 'cgi' interpreter cannot be executed : " + tokens[pos].value);
	location.cgi[cgi_extension] = tokens[pos].value;
	pos++;
	expect(tokens, pos, SEMICOLON);
	pos++;
}

void ConfigParser::parseReturn(LocationConfig &location, const std::vector<Token> &tokens, size_t &pos)
{
	pos++;
	expect(tokens, pos, IDENTIFIER);
	if (!isNumber(tokens[pos].value))
		error(tokens[pos], "First argument of 'return' must be a number");
	location.hasRedirect = true;
	location.redirectCode = std::atoi(tokens[pos].value.c_str());
	pos++;
	if (tokens[pos].type == SEMICOLON && location.redirectCode >= 300 && location.redirectCode <= 399)
		error(tokens[pos], "Return code between 300 and 399 must have a redirection path in second argument of 'return'");
	if (tokens[pos].type == IDENTIFIER)
	{
		location.redirectURL = tokens[pos].value;
		pos++;
	}
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
		if (i == 0 && s[i] == '-')
			continue;
		if (!std::isdigit(static_cast<unsigned char>(s[i])))
			return false;
	}
	return true;
}

bool ConfigParser::isIPv4(const std::string& s)
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

bool ConfigParser::isValidPort(const std::string& s)
{
	if (s.empty())
		return false;
	if (!isNumber(s))
		return false;
	int n = std::atoi(s.c_str());
	if (n < 1 || n > 65535)
		return false;
	return true;
}

bool ConfigParser::isDirectory(const std::string &path)
{
	DIR *dir = opendir(path.c_str());

    if (dir == NULL)
        return false;

    closedir(dir);
    return true;
}

ConfigParser::ConfigParser(const std::string &configFile)
{
	_serverParsers["listen"] = &ConfigParser::parseListen;
	_serverParsers["server_name"] = &ConfigParser::parseServerName;
	_serverParsers["client_max_body_size"] = &ConfigParser::parseClientMaxBodySize;
	_serverParsers["error_page"] = &ConfigParser::parseErrorPage;

	_locationParsers["root"] = &ConfigParser::parseRoot;
	_locationParsers["index"] = &ConfigParser::parseIndex;
	_locationParsers["allowed_methods"] = &ConfigParser::parseAllowedMethods;
	_locationParsers["autoindex"] = &ConfigParser::parseAutoIndex;
	_locationParsers["upload_dir"] = &ConfigParser::parseUploadDir;
	_locationParsers["cgi"] = &ConfigParser::parseCgi;
	_locationParsers["return"] = &ConfigParser::parseReturn;

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
