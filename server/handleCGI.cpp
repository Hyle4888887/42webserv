
#include "server.hpp"

// Start a CGI process and register its pipes.
void Server::startCGI(int clientFd, CGI &cgi)
{
    Client &client = _clients[clientFd];
    if (!cgi.start()) {
        client.outBuffer = errorFor(500, client.listenFd, client.cgiRequest);
        client.responseReady = true;
        setClientPollout(clientFd);
        return; }
    client.CGIActive = true; client.CGIPid = cgi.getPid();
    client.CGIFdOut = cgi.getFdOut(); client.CGIFdIn = cgi.getFdIn();
    client.CGIInput.swap(cgi.body); client.CGIInputOffset = 0;
    client.CGIOutput.clear(); client.CGIStart = std::time(NULL);
    struct pollfd p;
    p.fd = client.CGIFdOut; p.events = POLLIN; p.revents = 0;
    _pollFds.push_back(p); _CGIToClient[client.CGIFdOut] = clientFd;
    if (client.CGIFdIn != -1) {
        struct pollfd q;
        q.fd = client.CGIFdIn; q.events = POLLOUT; q.revents = 0;
        _pollFds.push_back(q); _CGIToClient[client.CGIFdIn] = clientFd;
    }
}
// Read output from the CGI process and store it for the response.
void Server::handleCGIRead(std::size_t index)
{
    int CGIFd = _pollFds[index].fd;
    std::map<int,int>::iterator m = _CGIToClient.find(CGIFd);
    if (m == _CGIToClient.end()) { _pollFds[index].fd = -1; return; }
    int clientFd = m->second;
    Client &client = _clients[clientFd];
    char buf[4096]; ssize_t n = read(CGIFd, buf, sizeof(buf));
    if (n > 0) { client.CGIOutput.append(buf, n); return; }
    if (client.CGIPid > 0) { waitpid(client.CGIPid, NULL, 0); client.CGIPid = -1; }
    _CGIToClient.erase(CGIFd); close(CGIFd);
    client.CGIFdOut = -1; _pollFds[index].fd = -1;
    if (client.CGIFdIn != -1) {
        _CGIToClient.erase(client.CGIFdIn);
        disablePollFdByFd(client.CGIFdIn);
        close(client.CGIFdIn);
        client.CGIFdIn = -1; }
    finishCGI(clientFd);
}
// Handle a CGI failure by killing it and sending a 502 response.
void Server::handleCGIError(std::size_t index)
{
    int CGIFd = _pollFds[index].fd;
    std::map<int,int>::iterator m = _CGIToClient.find(CGIFd);
    if (m == _CGIToClient.end()) { _pollFds[index].fd = -1; return; }
    int clientFd = m->second;
    Client &client = _clients[clientFd];
    if (client.CGIPid > 0) { kill(client.CGIPid, SIGKILL); waitpid(client.CGIPid, NULL, 0); client.CGIPid = -1; }
    _CGIToClient.erase(CGIFd);
    close(CGIFd);
    client.CGIFdOut = -1;
    _pollFds[index].fd = -1;
    if (client.CGIFdIn != -1) {
        _CGIToClient.erase(client.CGIFdIn);
        disablePollFdByFd(client.CGIFdIn);
        close(client.CGIFdIn);
        client.CGIFdIn = -1;
    }
    client.CGIActive = false;
    client.outBuffer = errorFor(502, client.listenFd, client.cgiRequest);
    client.responseReady = true;
    setClientPollout(clientFd);
}
// Forward request data to the CGI process stdin.
void Server::handleCGIWrite(std::size_t index)
{
    int CGIFd = _pollFds[index].fd;
    std::map<int,int>::iterator m = _CGIToClient.find(CGIFd);
    if (m == _CGIToClient.end()) { _pollFds[index].fd = -1; return; }
    Client &client = _clients[m->second];
    if (client.CGIInputOffset < client.CGIInput.size()) {
        ssize_t n = write(CGIFd, client.CGIInput.data() + client.CGIInputOffset,
                          client.CGIInput.size() - client.CGIInputOffset);
        if (n <= 0)
        {
            _CGIToClient.erase(CGIFd);
            close(CGIFd);
            client.CGIFdIn = -1;
            _pollFds[index].fd = -1;
        }
        else
            client.CGIInputOffset += static_cast<std::size_t>(n);
    }
    if (client.CGIInputOffset == client.CGIInput.size()) {
        _CGIToClient.erase(CGIFd); close(CGIFd);
        client.CGIFdIn = -1; client.CGIInput.clear(); client.CGIInputOffset = 0;
        _pollFds[index].fd = -1; }
}
// Finalize CGI processing and turn the output into an HTTP response.
void Server::finishCGI(int clientFd)
{
    std::map<int,Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end()) { return; }
    Client &c = it->second; c.CGIActive = false;
    std::string target, resp = CGI::buildResponse(c.CGIOutput, target);
    c.CGIOutput.clear();
    if (!target.empty()) {
        if (++c.CGIRedirects <= 5) {internalRedirect(clientFd, target); return; }
        resp = errorFor(500, c.listenFd, c.cgiRequest);
    } else if (resp.empty()) resp = errorFor(502, c.listenFd, c.cgiRequest);
    c.outBuffer = resp; c.responseReady = true;
    setClientPollout(clientFd);
}
// Trigger a local redirect while preserving the same client context.
void Server::internalRedirect(int clientFd, const std::string &target) {
    Request req = _clients[clientFd].cgiRequest;
    req.method = "GET"; req.body.clear(); req.headers.erase("content-lenght");
    req.headers.erase("content_type"); req.headers.erase("transfer-encoding");
    std::string::size_type q = target.find('?'); req.path = target.substr(0, q);
    req.query = (q == std::string::npos) ? std::string() : target.substr(q+1);
    dispatchRequest(clientFd, req);
}

// Tell whether a file descriptor belongs to an active CGI process.
bool Server::isCGIFd(int fd) const { return _CGIToClient.find(fd) != _CGIToClient.end(); }
// Detect the CGI script target from the request path and location config.
bool Server::findCgiTarget(const Request &req, const ServerConfig &config, std::string &interpreter, std::string &scriptName, std::string &pathInfo) const
{
	const LocationConfig *location = matchLocation(req.path, config);
	if (!location || location->cgi.empty()) return false;
	std::string::size_type end = 0;
	while (end != std::string::npos) {
		end = req.path.find('/', end + 1);
		std::string prefix = req.path.substr(0, end);
		std::string::size_type dot = prefix.rfind('.'), slash = prefix.rfind('/');
		if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
			continue;
		std::map<std::string, std::string>::const_iterator it = location->cgi.find(prefix.substr(dot));
		if (it != location->cgi.end()) {
			interpreter = it->second; scriptName = prefix;
			pathInfo = (end == std::string::npos) ? std::string() : req.path.substr(end);
			return true;
		}
	} return false;
}

// Resolve the server name for the CGI environment variables.
std::string Server::cgiServerName(const Request &req, const ServerConfig &cfg, int listenFd) const {
    std::map<std::string, std::string>::const_iterator h = req.headers.find("host");
    if (h != req.headers.end() && !h->second.empty())
        return h->second.substr(0, h->second.find(':'));
    if (!cfg.serverName.empty()) return cfg.serverName;
    std::map<int, std::string>::const_iterator l = _listenerHosts.find(listenFd);
    return l != _listenerHosts.end() ? l->second : std::string("localhost");
}