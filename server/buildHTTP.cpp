#include "server.hpp"

#include <cctype>

static std::string toLowerCopy(const std::string &s)
{
	std::string out = s;
	for (std::size_t i = 0; i < out.size(); ++i)
		out[i] = static_cast<char>(std::tolower(out[i]));
	return out;
}

static std::string trimCopy(const std::string &s)
{
	std::size_t begin = 0;
	while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin])))
		++begin;

	std::size_t end = s.size();
	while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1])))
		--end;

	return s.substr(begin, end - begin);
}

Request Server::parseRequest(const std::string &rawRequest) const
{
	Request req;
	req.path = "/";
	req.version = "HTTP/1.1";

	std::string::size_type lineEnd = rawRequest.find("\r\n");
	if (lineEnd != std::string::npos)
	{
		std::string requestLine = rawRequest.substr(0, lineEnd);
		std::istringstream firstLine(requestLine);
		std::string target;
		firstLine >> req.method >> target >> req.version;
		if (!target.empty())
		{
			req.path = target;
			std::string::size_type qPos = target.find('?');
			if (qPos != std::string::npos)
			{
				req.path = target.substr(0, qPos);
				req.query = target.substr(qPos + 1);
			}
		}
	}

	std::string::size_type headersStart = (lineEnd == std::string::npos) ? 0 : lineEnd + 2;
	std::string::size_type headersEnd = rawRequest.find("\r\n\r\n");
	if (headersEnd != std::string::npos && headersEnd >= headersStart)
	{
		std::size_t cursor = headersStart;
		while (cursor < headersEnd)
		{
			std::string::size_type next = rawRequest.find("\r\n", cursor);
			if (next == std::string::npos || next > headersEnd)
				break;
			std::string line = rawRequest.substr(cursor, next - cursor);
			std::string::size_type sep = line.find(':');
			if (sep != std::string::npos)
			{
				std::string key = toLowerCopy(trimCopy(line.substr(0, sep)));
				std::string value = trimCopy(line.substr(sep + 1));
				req.headers[key] = value;
			}
			cursor = next + 2;
		}
		req.body = rawRequest.substr(headersEnd + 4);
	}

	return req;
}

static std::string stripPort(const std::string &host)
{
	std::string::size_type colon = host.find(':');
	if (colon == std::string::npos)
		return host;
	return host.substr(0, colon);
}

const ServerConfig *Server::selectServerConfig(int listenFd, const Request &req) const
{
	if (_config.servers.empty())
		return NULL;

	int listenPort = -1;
	std::string listenHost;
	std::map<int, int>::const_iterator listenIt = _listenerPorts.find(listenFd);
	if (listenIt != _listenerPorts.end())
		listenPort = listenIt->second;
	std::map<int, std::string>::const_iterator listenerHostIt = _listenerHosts.find(listenFd);
	if (listenerHostIt != _listenerHosts.end())
		listenHost = listenerHostIt->second;

	std::string host;
	std::map<std::string, std::string>::const_iterator hostIt = req.headers.find("host");
	if (hostIt != req.headers.end())
		host = stripPort(hostIt->second);

	const ServerConfig *portMatch = NULL;
	for (std::vector<ServerConfig>::const_iterator it = _config.servers.begin(); it != _config.servers.end(); ++it)
	{
		if (listenPort != -1 && it->port != listenPort)
			continue;
		if (!host.empty() && !it->serverName.empty() && it->serverName == host)
			return &(*it);
		if (portMatch == NULL && (listenHost.empty() || listenHost == "0.0.0.0" || it->host == listenHost))
			portMatch = &(*it);
	}

	if (portMatch != NULL)
		return portMatch;
	return &_config.servers[0];
}

// Build an HTTP response from the parsed request.
void	Server::buildResponse(Client &client, const std::string &rawRequest)
{
	Request req = parseRequest(rawRequest);
	const ServerConfig *serverConfig = selectServerConfig(client.listenFd, req);
	if (serverConfig == NULL)
	{
		client.outBuffer = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
		return;
	}

	client.outBuffer = Response::build(req, *serverConfig);
}
