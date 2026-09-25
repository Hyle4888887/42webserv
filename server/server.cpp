
#include "server.hpp"

Server::Server()
{
}

Server::Server(const Config &config) : _config(config)
{
}

Server::~Server()
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		Client &client = it->second;
		if (client.CGIPid > 0)
		{
			kill(client.CGIPid, SIGKILL);
			waitpid(client.CGIPid, NULL, 0);
		}
		if (client.CGIFdIn != -1)
			close(client.CGIFdIn);
		if (client.CGIFdOut != -1)
			close(client.CGIFdOut);
		if (client.responseFileFd != -1)
			close(client.responseFileFd);
		close(it->first);
	}
	for (std::size_t i = 0; i < _listenFds.size(); ++i)
		if (_listenFds[i] != -1)
			close(_listenFds[i]);
}

// Run the main server loop.
void Server::run()
{
	if (_pollFds.empty())
	{
		std::cerr << "Aucun port en ecoute." << std::endl;
		return;
	}
 
	while (!g_stop)
	{
		int ready = poll(&_pollFds[0], static_cast<nfds_t>(_pollFds.size()), 5000);
 
		if (ready < 0)
		{
			if (errno == EINTR)
				break;
			std::cerr << "poll: " << std::strerror(errno) << std::endl;
			break;
		}
 
		checkTimeouts(); checkCGITimeouts();
 
		for (std::size_t i = _pollFds.size(); i-- > 0;)
		{
			short revents = _pollFds[i].revents;
			if (revents == 0)
				continue;
 
			int fd = _pollFds[i].fd;
			if (fd < 0 || revents == 0) { continue; }
			if (revents & (POLLERR | POLLNVAL))
			{
				if (isCGIFd(fd)) { handleCGIError(i); }
				else if (!isListener(fd))
					closeClient(i);
				continue;
			}
 
			if (isListener(fd))
				handleNewConnection(fd);
			else if (isCGIFd(fd))
			{
				if (revents & (POLLIN | POLLHUP)) { handleCGIRead(i); }
				else if (revents & POLLOUT) { handleCGIWrite(i); }
			}
			else if (revents & POLLIN)
				handleRead(i);

			else if (revents & POLLOUT)
				handleWrite(i);

			else if (!isListener(fd) && (revents & (POLLHUP | POLLRDHUP)))
			{
				handleRead(i);
			}
		}
		compactPollFds();
	}
}

// Close and remove a client connection.
void	Server::closeClient(std::size_t index)
{
	int	fd = _pollFds[index].fd;

	std::map<int, Client>::iterator	it = _clients.find(fd);
	if (it != _clients.end() && it->second.CGIActive)
	{
		Client	&c = it->second;
		if (c.CGIPid > 0)
		{
			kill(c.CGIPid, SIGKILL);
			waitpid(c.CGIPid, NULL, 0);
		}
		if (c.CGIFdOut != -1)
		{
			_CGIToClient.erase(c.CGIFdOut);
			disablePollFdByFd(c.CGIFdOut);
			close(c.CGIFdOut);
		}
		if (c.CGIFdIn != -1)
		{
			_CGIToClient.erase(c.CGIFdIn);
			disablePollFdByFd(c.CGIFdIn);
			close(c.CGIFdIn);
		}
	}
	if (it != _clients.end() && it->second.responseFileFd != -1)
		close(it->second.responseFileFd);

	close(fd);
	_clients.erase(fd);
	_pollFds[index].fd = -1;
}
