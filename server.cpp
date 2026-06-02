/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 12:51:56 by bozil             #+#    #+#             */
/*   Updated: 2026/06/02 13:09:40 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

namespace
{
	std::string	toString(unsigned long value)
	{
		std::ostringstream	oss;
		oss << value;
		return oss.str();
	}
}

Server::Server()
{
}

Server::~Server()
{
	for (std::size_t i = 0; i < _pollFds.size(); ++i)
		close(_pollFds[i].fd);
}

bool	Server::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl: " << std::strerror(errno) << std::endl;
		return false;
	}
	return true;
}

int	Server::ListeningSocket(int port)
{
	int	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		std::cerr << "socket: " << std::strerror(errno) << std::endl;
		return -1;
	}

	int	opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "setsockopt: " << std::strerror(errno) << std::endl;
		close(fd);
		return -1;
	}

	struct sockaddr_in	addr = {};
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(static_cast<unsigned short>(port));

	if (bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
	{
		std::cerr << "bind (port " << port << "): "
				  << std::strerror(errno) << std::endl;
		close(fd);
		return -1;
	}
	if (listen(fd, 128) < 0)
	{
		std::cerr << "listen: " << std::strerror(errno) << std::endl;
		close(fd);
		return -1;
	}
	if (!setNonBlocking(fd))
	{
		close(fd);
		return -1;
	}
	return fd;
}

bool	Server::addListener(int port)
{
	int	fd = ListeningSocket(port);
	if (fd < 0)
		return false;

	struct pollfd	pfd;
	pfd.fd      = fd;
	pfd.events  = POLLIN;	// read
	pfd.revents = 0;
	_pollFds.push_back(pfd);
	_listenFds.push_back(fd);

	std::cout << "Listening on port " << port << " (fd=" << fd << ")"
			  << std::endl;
	return true;
}

bool	Server::isListener(int fd) const
{
	for (std::size_t i = 0; i < _listenFds.size(); ++i)
		if (_listenFds[i] == fd)
			return true;
	return false;
}

void	Server::run()
{
	if (_pollFds.empty())
	{
		std::cerr << "Aucun port en ecoute, rien a faire." << std::endl;
		return;
	}

	while (true)
	{
		int	ready = poll(&_pollFds[0], static_cast<nfds_t>(_pollFds.size()), -1);
		if (ready < 0)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "poll: " << std::strerror(errno) << std::endl;
			break;
		}

		for (std::size_t i = _pollFds.size(); i-- > 0; )
		{
			short	revents = _pollFds[i].revents;
			if (revents == 0)
				continue;

			int	fd = _pollFds[i].fd;

			if (revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (!isListener(fd))
					closeClient(i);
				continue;
			}

			if (isListener(fd))
				handleNewConnection(fd);
			else if (revents & POLLIN)
				handleRead(i);
			else if (revents & POLLOUT)
				handleWrite(i);
		}
	}
}

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
		pfd.events  = POLLIN;	// wait
		pfd.revents = 0;
		_pollFds.push_back(pfd);
		_clients[clientFd] = Client();

		std::cout << "[+] Client connecte fd=" << clientFd << std::endl;
	}
}

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

	Client	&client = _clients[fd];
	client.inBuffer.append(buffer, static_cast<std::size_t>(n));

	/* parsing here.)*/
	if (!client.responseReady
		&& client.inBuffer.find("\r\n\r\n") != std::string::npos)
	{
		buildResponse(client);
		client.responseReady = true;
		_pollFds[index].events = POLLOUT;
	}
}

void	Server::buildResponse(Client &client)
{
	std::string	body = "<h1>Hello from webserv</h1>";

	std::string	response;
	response  = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + toString(body.size()) + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += body;

	client.outBuffer = response;
}

void	Server::handleWrite(std::size_t index)
{
	int		fd = _pollFds[index].fd;
	Client	&client = _clients[fd];

	ssize_t	n = send(fd, client.outBuffer.c_str(), client.outBuffer.size(), 0);
	if (n <= 0)
	{
		closeClient(index);
		return;
	}
	client.outBuffer.erase(0, static_cast<std::size_t>(n));
	if (client.outBuffer.empty())
	{
		std::cout << "[i] Reponse envoyee, fermeture fd=" << fd << std::endl;
		closeClient(index);	// Connection: close
	}
}

void	Server::closeClient(std::size_t index)
{
	int	fd = _pollFds[index].fd;

	close(fd);
	_clients.erase(fd);
	_pollFds.erase(_pollFds.begin() + index);
}
