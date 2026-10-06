#include "server.hpp"

#include <cctype>

// Check whether a version string matches an HTTP/X.Y format.
static bool isHTTPformat(const std::string &v)
{
	return v.size() == 8 && v.compare(0, 5, "HTTP/") == 0 
	&& std::isdigit(static_cast<unsigned char>(v[5])) 
	&& v[6] == '.' 
	&& std::isdigit(static_cast<unsigned char>(v[7]));
}

// Collapse repeated slashes so "//directory" matches the "/directory" location.
static std::string collapseSlashes(const std::string &path)
{
	std::string out;
	out.reserve(path.size());
	for (std::size_t i = 0; i < path.size(); ++i)
	{
		if (path[i] == '/' && !out.empty() && out[out.size() - 1] == '/')
			continue;
		out += path[i];
	}
	if (out.empty())
		out = "/";
	return out;
}

// Parse a raw HTTP request into a structured Request object.
Request Server::parseRequest(const std::string &rawRequest) const
{
	Request req;
	req.path = "/";
	req.version = "HTTP/1.1";
	req.valid = false;

	std::string::size_type lineEnd = rawRequest.find("\r\n");
	if (lineEnd == std::string::npos)
		return req;

	std::string requestLine = rawRequest.substr(0, lineEnd);
	std::istringstream firstLine(requestLine);
	std::string target;
	std::string extra;
	if (!(firstLine >> req.method >> target >> req.version) || (firstLine >> extra))
		return req;
	if (target.empty() || !isHTTPformat(req.version))
	{
		return req;
	}
	if (req.version != "HTTP/1.1")
	{
		req.errorCode = 505;
		return req;
	}

	req.valid = true;
	if (!target.empty())
	{
		req.path = target;
		std::string::size_type qPos = target.find('?');
		if (qPos != std::string::npos)
		{
			req.path = target.substr(0, qPos);
			req.query = target.substr(qPos + 1);
		}
		req.path = collapseSlashes(req.path);
	}

	std::string::size_type headersStart = lineEnd + 2;
	std::string::size_type headersEnd = rawRequest.find("\r\n\r\n");
	if (headersEnd == std::string::npos || headersEnd < headersStart)
		return req;

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

	std::map<std::string, std::string>::const_iterator hostIt = req.headers.find("host");
	if (hostIt == req.headers.end() || trimCopy(hostIt->second).empty())
	{
		req.valid = false;
		return req;
	}

	req.body = rawRequest.substr(headersEnd + 4);
	return req;
}

// Remove the port part from a Host header value.
static std::string stripPort(const std::string &host)
{
	std::string::size_type colon = host.find(':');
	if (colon == std::string::npos)
		return host;
	return host.substr(0, colon);
}

// Choose the server block matching the listening socket and Host header.
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

// Build an error response for the selected virtual server.
std::string Server::errorFor(int code, int listenFd, const Request &req) const
{
	const ServerConfig *cfg = selectServerConfig(listenFd, req);
	if (cfg) return Response::errorResponse(code, *cfg);
	ServerConfig none; return Response::errorResponse(code, none);
}

// Build an HTTP response from the raw request text.
void	Server::buildResponse(Client &client, const std::string &rawRequest)
{
	Request req = parseRequest(rawRequest);
	buildResponse(client, req);
}

// Build an HTTP response from a parsed request object.
void	Server::buildResponse(Client &client, const Request &req)
{
	const ServerConfig *serverConfig = selectServerConfig(client.listenFd, req);
	if (serverConfig == NULL)
	{
		ServerConfig none;
		client.outBuffer = Response::errorResponse(500, none);
		return;
	}
	if (req.method == "GET" && Response::prepareDownload(req, *serverConfig,
		client.responseFileFd, client.responseFileRemaining, client.outBuffer))
		return;

	client.outBuffer = Response::build(req, *serverConfig);
}
