/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 13:54:14 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/09 13:57:31 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

/*rend le socket non bloquant*/
bool	Server::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl: " << std::strerror(errno) << std::endl;
		return false;
	}
	return true;
}

/*configure et crée un socket d'écoute*/
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

	struct sockaddr_in	addr;
	std::memset(&addr, 0, sizeof(addr));
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

/*crée un socket d'écoute*/
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

/*si fd est un socket d'écoute renvoie true*/
bool	Server::isListener(int fd) const
{
	for (std::size_t i = 0; i < _listenFds.size(); ++i)
		if (_listenFds[i] == fd)
			return true;
	return false;
}
