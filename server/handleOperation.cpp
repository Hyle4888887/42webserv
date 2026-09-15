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

static bool methodAllowed(const Request &req, const LocationConfig &location)
{
	const std::string &method = req.method;
	for (std::vector<std::string>::const_iterator it = location.allowedMethods.begin(); it != location.allowedMethods.end(); ++it)
		if (*it == method)
			return true;
	return location.allowedMethods.empty();
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

	if (client.outBufferOffset < client.outBuffer.size())
	{
		ssize_t n = send(fd, client.outBuffer.c_str() + client.outBufferOffset,
			client.outBuffer.size() - client.outBufferOffset, 0);
		if (n < 0)
		{
			std::cerr << "send error fd=" << fd << std::endl;
			closeClient(index);
			return;
		}
		if (n == 0)
		{
			closeClient(index);
			return;
		}
		client.outBufferOffset += static_cast<std::size_t>(n);
		if (client.outBufferOffset == client.outBuffer.size())
		{
			client.outBuffer.clear();
			client.outBufferOffset = 0;
		}
		client.lastActivityTime = std::time(NULL);
	}

	if (client.outBuffer.empty() && client.responseFileFd != -1)
	{
		char buffer[65536];
		std::size_t requested = client.responseFileRemaining < sizeof(buffer)
			? static_cast<std::size_t>(client.responseFileRemaining) : sizeof(buffer);
		ssize_t n = read(client.responseFileFd, buffer, requested);
		if (n < 0)
		{
			std::cerr << "file read error fd=" << fd << std::endl;
			closeClient(index);
			return;
		}
		if (n == 0)
		{
			closeClient(index);
			return;
		}
		client.outBuffer.assign(buffer, static_cast<std::size_t>(n));
		client.outBufferOffset = 0;
		client.responseFileRemaining -= static_cast<unsigned long long>(n);
		return;
	}

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

	std::string::size_type headersEnd;
	if (client.requestInitialized)
		headersEnd = client.requestHeaderEnd;
	else
	{
		headersEnd = client.inBuffer.find("\r\n\r\n");
		if (headersEnd == std::string::npos)
			return;
	}

	std::string headersBlock = client.inBuffer.substr(0, headersEnd);
	std::size_t contentLength = 0;
	bool hasContentLength = getContentLength(headersBlock, contentLength);
	std::string lowerHeaders = headersBlock;
	for (std::size_t i = 0; i < lowerHeaders.size(); ++i)
		lowerHeaders[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(lowerHeaders[i])));
	bool chunked = lowerHeaders.find("transfer-encoding:") != std::string::npos
		&& lowerHeaders.find("chunked") != std::string::npos;
	if (!client.requestInitialized)
	{
		client.requestInitialized = true;
		client.requestHeaderEnd = headersEnd;
		client.requestChunked = chunked;
		client.requestBodyCursor = headersEnd + 4;
		client.requestChunkRemaining = 0;
		client.requestNeedChunkCRLF = false;
		client.requestFinalCRLFPending = false;
		client.requestComplete = !chunked;
		client.requestBody.clear();
	}
	std::size_t maxBodySize = 0;
	bool bodyLimitKnown = false;
	Request limitRequest = parseRequest(client.inBuffer.substr(0, headersEnd + 4));
	const ServerConfig *limitConfig = selectServerConfig(client.listenFd, limitRequest);
	const LocationConfig *limitLocation = limitConfig == NULL ? NULL : matchLocation(limitRequest.path, *limitConfig);
	if (limitConfig != NULL)
	{
		maxBodySize = limitConfig->clientMaxBodySize;
		if (limitLocation != NULL && limitLocation->hasClientMaxBodySize)
			maxBodySize = limitLocation->clientMaxBodySize;
		bodyLimitKnown = true;
	}
	if (chunked && client.inBuffer.size() == client.requestBodyCursor)
	{
		Request headerRequest = parseRequest(client.inBuffer.substr(0, headersEnd + 4));
		const ServerConfig *headerConfig = selectServerConfig(client.listenFd, headerRequest);
		const LocationConfig *headerLocation = headerConfig == NULL ? NULL : matchLocation(headerRequest.path, *headerConfig);
		std::string headerInterpreter;
		std::string headerScript;
		bool headerIsCgi = headerConfig != NULL && findCgiTarget(headerRequest, *headerConfig, headerInterpreter, headerScript);
		bool headerMethodAllowed = headerLocation != NULL && methodAllowed(headerRequest, *headerLocation);
		if (!headerIsCgi && !headerMethodAllowed)
		{
			buildResponse(client, client.inBuffer.substr(0, headersEnd + 4));
			client.responseReady = true;
			_pollFds[index].events = POLLOUT;
			_pollFds[index].events |= POLLRDHUP;
			return;
		}
	}
	if (chunked)
	{
		while (!client.requestComplete)
		{
			if (client.requestFinalCRLFPending)
			{
				if (client.inBuffer.size() < client.requestBodyCursor + 2)
					return;
				if (client.inBuffer.compare(client.requestBodyCursor, 2, "\r\n") != 0)
					return;
				client.requestBodyCursor += 2;
				client.requestFinalCRLFPending = false;
				client.requestComplete = true;
				break;
			}
			if (client.requestNeedChunkCRLF)
			{
				if (client.inBuffer.size() < client.requestBodyCursor + 2)
					return;
				if (client.inBuffer.compare(client.requestBodyCursor, 2, "\r\n") != 0)
					return;
				client.requestBodyCursor += 2;
				client.requestNeedChunkCRLF = false;
			}
			if (client.requestChunkRemaining == 0)
			{
				std::string::size_type lineEnd = client.inBuffer.find("\r\n", client.requestBodyCursor);
				if (lineEnd == std::string::npos)
					return;
				std::string sizeText = client.inBuffer.substr(client.requestBodyCursor, lineEnd - client.requestBodyCursor);
				std::string::size_type extension = sizeText.find(';');
				if (extension != std::string::npos)
					sizeText.erase(extension);
				char *end = NULL;
				unsigned long chunkSize = std::strtoul(sizeText.c_str(), &end, 16);
				if (end == sizeText.c_str() || *end != '\0')
					return;
				client.requestBodyCursor = lineEnd + 2;
				if (chunkSize == 0)
				{
					client.requestFinalCRLFPending = true;
					continue;
				}
				client.requestChunkRemaining = static_cast<std::size_t>(chunkSize);
			}
			std::size_t available = client.inBuffer.size() - client.requestBodyCursor;
			std::size_t take = available < client.requestChunkRemaining ? available : client.requestChunkRemaining;
			if (take == 0)
				return;
			if (bodyLimitKnown && (client.requestBody.size() > maxBodySize
				|| take > maxBodySize - client.requestBody.size()))
			{
				client.outBuffer = "HTTP/1.1 413 Payload Too Large\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
				client.responseReady = true;
				_pollFds[index].events = POLLOUT;
				_pollFds[index].events |= POLLRDHUP;
				return;
			}
			client.requestBody.append(client.inBuffer, client.requestBodyCursor, take);
			client.requestBodyCursor += take;
			client.requestChunkRemaining -= take;
			if (client.requestChunkRemaining != 0)
				return;
			client.requestNeedChunkCRLF = true;
		}
	}
	if (hasContentLength && !chunked)
	{
		Request headersRequest = parseRequest(client.inBuffer.substr(0, headersEnd + 4));
		const ServerConfig *headersConfig = selectServerConfig(client.listenFd, headersRequest);
		const LocationConfig *headersLocation = headersConfig == NULL ? NULL : matchLocation(headersRequest.path, *headersConfig);
		std::size_t contentLengthLimit = headersConfig == NULL ? 0 : headersConfig->clientMaxBodySize;
		if (headersLocation != NULL && headersLocation->hasClientMaxBodySize)
			contentLengthLimit = headersLocation->clientMaxBodySize;
		if (headersConfig != NULL && contentLength > contentLengthLimit)
		{
			client.outBuffer = "HTTP/1.1 413 Payload Too Large\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
			client.responseReady = true;
			_pollFds[index].events = POLLOUT;
			_pollFds[index].events |= POLLRDHUP;
			return;
		}
	}

	std::size_t totalNeeded = chunked ? client.requestBodyCursor : headersEnd + 4 + contentLength;
	if (client.inBuffer.size() < totalNeeded)
		return;

	Request req;
	if (chunked)
	{
		std::string rawHeaders = client.inBuffer.substr(0, headersEnd + 4);
		client.inBuffer.erase(0, totalNeeded);
		req = parseRequest(rawHeaders);
		req.body.swap(client.requestBody);
	}
	else
	{
		std::string rawHeaders;
		rawHeaders.swap(client.inBuffer);
		req = parseRequest(rawHeaders.substr(0, headersEnd + 4));
		req.body.swap(rawHeaders);
		req.body.erase(0, headersEnd + 4);
		if (req.body.size() > contentLength)
		{
			client.inBuffer.assign(req.body, contentLength, req.body.size() - contentLength);
			req.body.erase(contentLength);
		}
	}

	std::string interpreter;
	std::string scriptPath;
	const ServerConfig *serverConfig = selectServerConfig(client.listenFd, req);
	const LocationConfig *location = serverConfig == NULL ? NULL : matchLocation(req.path, *serverConfig);
	bool CGI = (serverConfig != NULL && location != NULL && methodAllowed(req, *location)
		&& findCgiTarget(req, *serverConfig, interpreter, scriptPath));
	if (CGI && scriptPath == location->root)
		scriptPath = joinPath(location->root, req.path.substr(req.path.find_last_of('/') + 1));

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
		{
			std::string requestUri = req.path;
			if (!req.query.empty())
				requestUri += "?" + req.query;
			startCGI(fd, interpreter, scriptPath, req.method, req.query, requestUri, req.body,
				req.headers, serverConfig->serverName,
				toString(static_cast<unsigned long>(_listenerPorts[client.listenFd])));
			if (client.CGIActive)
				_pollFds[index].events = 0;
		}
	}
	else
	{
		buildResponse(client, req);
		client.responseReady = true;
		_pollFds[index].events = POLLOUT;
		_pollFds[index].events |= POLLRDHUP;
	}
}