
#include "server.hpp"

// Put a socket into non-blocking mode.
bool	Server::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl: " << std::strerror(errno) << std::endl;
		return false;
	}
	return true;
}

// Configure and bind a listening socket.
int	Server::ListeningSocket(const std::string &host, int port)
{
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	std::string portStr = toString(static_cast<unsigned long>(port));
	const char *node = (host.empty() || host == "0.0.0.0") ? NULL : host.c_str();
	struct addrinfo *result = NULL;
	int gai = getaddrinfo(node, portStr.c_str(), &hints, &result);
	if (gai != 0)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(gai) << std::endl;
		return -1;
	}

	int fd = -1;
	for (struct addrinfo *it = result; it != NULL; it = it->ai_next)
	{
		fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
		if (fd < 0)
			continue;

		int opt = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			close(fd);
			fd = -1;
			continue;
		}
		if (bind(fd, it->ai_addr, it->ai_addrlen) < 0)
		{
			close(fd);
			fd = -1;
			continue;
		}
		if (listen(fd, 128) < 0)
		{
			close(fd);
			fd = -1;
			continue;
		}
		if (!setNonBlocking(fd))
		{
			close(fd);
			fd = -1;
			continue;
		}
		break;
	}
	freeaddrinfo(result);
	if (fd < 0)
		std::cerr << "Unable to bind listener on " << host << ':' << port << std::endl;
	return fd;
}

// Add a listening socket to the poll set.
bool	Server::addListener(const std::string &host, int port)
{
	int	fd = ListeningSocket(host, port);
	if (fd < 0)
		return false;

	struct pollfd	pfd;
	pfd.fd      = fd;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	_pollFds.push_back(pfd);
	_listenFds.push_back(fd);
	_listenerHosts[fd] = host;
	_listenerPorts[fd] = port;

	std::cout << "Listening on " << host << ':' << port << " (fd=" << fd << ")"
			  << std::endl;
	return true;
}

// Check whether a file descriptor belongs to a listening socket.
bool	Server::isListener(int fd) const
{
	for (std::size_t i = 0; i < _listenFds.size(); ++i)
		if (_listenFds[i] == fd)
			return true;
	return false;
}
