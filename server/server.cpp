/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 12:51:56 by bozil             #+#    #+#             */
/*   Updated: 2026/06/10 11:07:51 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

Server::Server()
{
	RouteConfig rootRoute;
	rootRoute.root = ".";
	rootRoute.index = "index.html";
	rootRoute.dirListing = true;
	rootRoute.allowedMethods.push_back("GET");
	rootRoute.allowedMethods.push_back("POST");
	rootRoute.allowedMethods.push_back("DELETE");
	_config.routes["/"] = rootRoute;
}

Server::~Server()
{
	for (std::size_t i = 0; i < _pollFds.size(); ++i)
		close(_pollFds[i].fd);
}

/*fonction principale*/
void Server::run()
{
	if (_pollFds.empty())
	{
		std::cerr << "Aucun port en ecoute." << std::endl;
		return;
	}
 
	while (true)
	{
	/*premier timout pour voir les clients inactif*/
		int ready = poll(&_pollFds[0], static_cast<nfds_t>(_pollFds.size()), 5000); // en ms
 
		if (ready < 0)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "poll: " << std::strerror(errno) << std::endl;
			break;
		}
 
		/*reverifier les timeouts*/
		checkTimeouts(); checkCGITimeouts();
 
		/* Effacer les clients inactifs sans invalidé les indices restants */
		for (std::size_t i = _pollFds.size(); i-- > 0;)
		{
			short revents = _pollFds[i].revents;
			if (revents == 0)
				continue;
 
			int fd = _pollFds[i].fd;
			if (fd < 0 || revents == 0) { continue; }
			// Erreur ou deconnexion
			if (revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (isCGIFd(fd)) { handleCGIRead(i); }
				else if (!isListener(fd))
					closeClient(i);
				continue;
			}
 
			if (!isListener(fd) && (revents & POLLRDHUP))
			{
				std::cout << "[-] POLLRDHUP fd=" << fd << std::endl;
				closeClient(i);
				continue;
			}
 
			if (isListener(fd))
				handleNewConnection(fd);
			else if (isCGIFd(fd))
			{
				if (revents & POLLIN) { handleCGIRead(i); }
				else if (revents & POLLOUT) { handleCGIWrite(i); }
			}
			else if (revents & POLLIN)
				handleRead(i);

			else if (revents & POLLOUT)
				handleWrite(i);
			compactPollFds();
		}
	}
}

/*supprime un client */
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

	close(fd);
	_clients.erase(fd);
	_pollFds[index].fd = -1;
}
