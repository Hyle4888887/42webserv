
#include "server.hpp"

void Server::checkCGITimeouts()
{
    time_t now = std::time(NULL);
    for (std::map<int,Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        Client &c = it->second;
        if (!c.CGIActive || std::difftime(now, c.CGIStart) < CGI_TIMEOUT) continue;

        if (c.CGIPid > 0) { kill(c.CGIPid, SIGKILL); waitpid(c.CGIPid, NULL, 0); c.CGIPid = -1; }
        if (c.CGIFdOut != -1) { _CGIToClient.erase(c.CGIFdOut); disablePollFdByFd(c.CGIFdOut); close(c.CGIFdOut); c.CGIFdOut = -1; }
        if (c.CGIFdIn  != -1) { _CGIToClient.erase(c.CGIFdIn);  disablePollFdByFd(c.CGIFdIn);  close(c.CGIFdIn);  c.CGIFdIn  = -1; }
        c.CGIActive = false;
		c.outBuffer = "HTTP/1.1 504 Gateway Timeout\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        c.responseReady = true;
        setClientPollout(it->first);
    }
}

// Close clients that have been idle for too long.
void Server::checkTimeouts()
{
	time_t now = std::time(NULL);
 
	for (std::size_t i = _pollFds.size(); i-- > 0;)
	{
		int fd = _pollFds[i].fd;
 
		if (isListener(fd))
			continue;
 
		std::map<int, Client>::iterator it = _clients.find(fd);
		if (it == _clients.end())
			continue;
 
		double elapsed = std::difftime(now, it->second.lastActivityTime);
		if (elapsed > CLIENT_TIMEOUT)
		{
			std::cout << "[!] Timeout client fd=" << fd
					  << " (inactif " << (int)elapsed << "s)" << std::endl;
			closeClient(i);
		}
	}
}
