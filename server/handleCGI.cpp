
#include "server.hpp"

// Start a CGI process and register its pipes.
void Server::startCGI(int clientFd, const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &body)
{
    Client &client = _clients[clientFd];
    CGI *cgi = new CGI();
    if (!cgi->start(interpreter, scriptPath, method, query, body)) {
        delete cgi;
        client.outBuffer = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        client.responseReady = true;
        setClientPollout(clientFd);
        return; }

    client.CGIActive = true; client.CGIPid    = cgi->getPid();
    client.CGIFdOut  = cgi->getFdOut(); client.CGIFdIn   = cgi->getFdIn();
    client.CGIInput  = body; client.CGIOutput.clear();
    client.CGIStart  = std::time(NULL);
    delete cgi;
    struct pollfd p;
    p.fd = client.CGIFdOut; p.events = POLLIN; p.revents = 0;
    _pollFds.push_back(p); _CGIToClient[client.CGIFdOut] = clientFd;
    if (client.CGIFdIn != -1) {
        struct pollfd q;
        q.fd = client.CGIFdIn; q.events = POLLOUT; q.revents = 0;
        _pollFds.push_back(q); _CGIToClient[client.CGIFdIn] = clientFd;
    }
}
void Server::handleCGIRead(std::size_t index)
{
    int CGIFd = _pollFds[index].fd;
    std::map<int,int>::iterator m = _CGIToClient.find(CGIFd);
    if (m == _CGIToClient.end()) { _pollFds[index].fd = -1; return; }
    Client &client = _clients[m->second];
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
    finishCGI(m->second);
}
void Server::handleCGIWrite(std::size_t index)
{
    int CGIFd = _pollFds[index].fd;
    std::map<int,int>::iterator m = _CGIToClient.find(CGIFd);
    if (m == _CGIToClient.end()) { _pollFds[index].fd = -1; return; }
    Client &client = _clients[m->second];
    while (!client.CGIInput.empty()) {
        ssize_t n = write(CGIFd, client.CGIInput.c_str(), client.CGIInput.size());
        if (n <= 0)
            break;
        client.CGIInput.erase(0, static_cast<std::size_t>(n));
    }
    if (client.CGIInput.empty()) {
        _CGIToClient.erase(CGIFd); close(CGIFd);
        client.CGIFdIn = -1; _pollFds[index].fd = -1; }
}
void Server::finishCGI(int clientFd)
{
    std::map<int,Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end()) { return; }
    it->second.outBuffer = CGI::buildResponse(it->second.CGIOutput);
    it->second.responseReady = true; it->second.CGIActive = false;
    setClientPollout(clientFd);
}
bool Server::isCGIFd(int fd) const { return _CGIToClient.find(fd) != _CGIToClient.end(); }
