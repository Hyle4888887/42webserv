#include "server.hpp"

static bool getContentLength(const std::string &headersBlock, std::size_t &length)
{
	std::string lower = headersBlock;
	for (std::size_t i = 0; i < lower.size(); ++i)
		lower[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(lower[i])));

	std::string::size_type pos = lower.find("content-length:");
	if (pos == std::string::npos)
		return false;
	pos += 16; // length of "content-length:"

	std::string::size_type lineEnd = headersBlock.find("\r\n", pos);
	if (lineEnd == std::string::npos)
		lineEnd = headersBlock.size();

	std::string value = headersBlock.substr(pos, lineEnd - pos);
	std::string::size_type b = 0, e = value.size();
	while (b < e && std::isspace(static_cast<unsigned char>(value[b])))
		++b;
	while (e > b && std::isspace(static_cast<unsigned char>(value[e - 1])))
		--e;
	value = value.substr(b, e - b);
	if (value.empty())
		return false;

	length = static_cast<std::size_t>(std::atol(value.c_str()));
	return true;
}

static std::string joinPath(const std::string &base, const std::string &suffix)
{
	if (base.empty())
		return suffix;
	if (suffix.empty())
		return base;
	if (base[base.size() - 1] == '/' && suffix[0] == '/')
		return base + suffix.substr(1);
	if (base[base.size() - 1] != '/' && suffix[0] != '/')
		return base + "/" + suffix;
	return base + suffix;
}

const LocationConfig *Server::matchLocation(const std::string &path, const ServerConfig &config) const
{
	const LocationConfig *best = NULL;
	std::size_t bestLen = 0;

	for (std::vector<LocationConfig>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it)
	{
		const std::string &prefix = it->path;
		if (prefix.empty())
			continue;
		if (path.compare(0, prefix.size(), prefix) == 0
			&& prefix.size() >= bestLen
			&& (prefix == "/" || path.size() == prefix.size() || path[prefix.size()] == '/'))
		{
			best = &(*it);
			bestLen = prefix.size();
		}
	}
	return best;
}

std::string Server::resolvePath(const std::string &urlPath, const LocationConfig &location) const
{
	std::string::size_type pos = 0;
	while ((pos = urlPath.find("..", pos)) != std::string::npos)
	{
		bool before = (pos == 0 || urlPath[pos - 1] == '/');
		bool after  = (pos + 2 == urlPath.size() || urlPath[pos + 2] == '/');
		if (before && after)
			return "";
		pos += 2;
	}

	std::string fs = location.root;
	if (!fs.empty() && lastC(fs) == '/')
		fs.erase(fs.size() - 1);
	std::string suffix = urlPath;
	if (urlPath.compare(0, location.path.size(), location.path) == 0)
		suffix = urlPath.substr(location.path.size());
	fs = joinPath(fs, suffix);
	return fs;
}

bool Server::findCgiTarget(const Request &req, const ServerConfig &config, std::string &interpreter, std::string &scriptPath) const
{
	const LocationConfig *location = matchLocation(req.path, config);
	if (!location)
		return false;

	std::string::size_type dot = req.path.rfind('.');
	if (dot == std::string::npos)
		return false;

	std::string extension = req.path.substr(dot);
	std::map<std::string, std::string>::const_iterator it = location->cgi.find(extension);
	if (it == location->cgi.end())
		return false;

	interpreter = it->second;
	scriptPath = resolvePath(req.path, *location);
	return !scriptPath.empty();
}

// Handle new incoming client connections.
void	Server::handleNewConnection(int listenFd)
{
	while (true)
	{
		int	clientFd = accept(listenFd, NULL, NULL);
		if (clientFd < 0)
			break;

		if (!setNonBlocking(clientFd))
		{
			close(clientFd);
			continue;
		}

		struct pollfd	pfd;
		pfd.fd      = clientFd;
		pfd.events  = POLLIN;
		pfd.events |= POLLRDHUP;
		pfd.revents = 0;
		_pollFds.push_back(pfd);
		_clients[clientFd] = Client();
		_clients[clientFd].listenFd = listenFd;

		std::cout << "[+] Client connecte fd=" << clientFd << std::endl;
	}
}

// Send the prepared response to the client.
void	Server::handleWrite(std::size_t index)
{
	int		fd = _pollFds[index].fd;
	Client	&client = _clients[fd];

	ssize_t	n = send(fd, client.outBuffer.c_str(), client.outBuffer.size(), 0);
	if (n <= 0)
	{
		std::cerr << "send error fd=" << fd << std::endl;
		closeClient(index);
		return;
	}
 
	client.outBuffer.erase(0, static_cast<std::size_t>(n));
	client.lastActivityTime = std::time(NULL);
 
	if (client.outBuffer.empty())
	{
		std::cout << "[i] Reponse envoyee fd=" << fd << std::endl;
		closeClient(index);
	}
}
// Read incoming client data and process it once a full request has arrived.
void	Server::handleRead(std::size_t index)
{
	int		fd = _pollFds[index].fd;
	char	buffer[4096];

	ssize_t	n = recv(fd, buffer, sizeof(buffer), 0);

	if (n <= 0)
	{
		std::cout << "[-] Client deconnecte fd=" << fd << std::endl;
		closeClient(index);
		return;
	}

	Client &client = _clients[fd];
	client.inBuffer.append(buffer, static_cast<std::size_t>(n));
	client.lastActivityTime = std::time(NULL);

	std::string::size_type headersEnd = client.inBuffer.find("\r\n\r\n");
	if (headersEnd == std::string::npos)
		return;

	std::string headersBlock = client.inBuffer.substr(0, headersEnd);
	std::size_t contentLength = 0;
	getContentLength(headersBlock, contentLength);

	std::size_t totalNeeded = headersEnd + 4 + contentLength;
	if (client.inBuffer.size() < totalNeeded)
		return;

	std::string rawRequest = client.inBuffer.substr(0, totalNeeded);
	client.inBuffer.erase(0, totalNeeded);

	Request req = parseRequest(rawRequest);

	std::string interpreter;
	std::string scriptPath;
	const ServerConfig *serverConfig = selectServerConfig(client.listenFd, req);
	bool CGI = (serverConfig != NULL && findCgiTarget(req, *serverConfig, interpreter, scriptPath));

	if (CGI)
	{
		if (access(scriptPath.c_str(), R_OK) != 0)
		{
			client.outBuffer = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
			client.responseReady = true;
			_pollFds[index].events = POLLOUT;
			_pollFds[index].events |= POLLRDHUP;
		}
		else
			startCGI(fd, interpreter, scriptPath, req.method, req.query, req.body);
	}
	else
	{
		buildResponse(client, rawRequest);
		client.responseReady = true;
		_pollFds[index].events = POLLOUT;
		_pollFds[index].events |= POLLRDHUP;
	}
}