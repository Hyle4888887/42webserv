/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 12:52:06 by bozil             #+#    #+#             */
/*   Updated: 2026/06/02 15:10:12 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <cerrno>
# include <cstddef>
# include <cstring>
# include <fcntl.h>
# include <iostream>
# include <map>
# include <netinet/in.h>
# include <poll.h>
# include <sstream>
# include <string>
# include <sys/socket.h>
# include <unistd.h>
# include <vector>

class Server
{
  public:
	Server();
	~Server();
	bool addListener(int port);
	void run();

  private:
	Server(const Server &other);
	Server &operator=(const Server &other);
	struct	Client
	{
		std::string inBuffer;  // get data
		std::string outBuffer; // to send
		bool responseReady;    // send

		Client() : responseReady(false)
		{
		}
	};

	bool setNonBlocking(int fd);
	int ListeningSocket(int port);
	bool isListener(int fd) const;

	void handleNewConnection(int listenFd);
	void handleRead(std::size_t index);
	void handleWrite(std::size_t index);
	void closeClient(std::size_t index);
	void buildResponse(Client &client);

	std::vector<int> _listenFds;
	std::vector<struct pollfd> _pollFds;
	std::map<int, Client> _clients;
};

#endif
