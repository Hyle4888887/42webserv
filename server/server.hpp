/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 12:52:06 by bozil             #+#    #+#             */
/*   Updated: 2026/06/04 13:55:01 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <ctime>
#include <cstddef>
#include <map>
#include <poll.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <iostream>

#ifndef POLLRDHUP
# define POLLRDHUP 0
#endif

#define CLIENT_TIMEOUT 20

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
		time_t lastActivityTime; // timeout

		Client() : responseReady(false), lastActivityTime(std::time(NULL))
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
	void checkTimeouts();

	std::vector<int> _listenFds;
	std::vector<struct pollfd> _pollFds;
	std::map<int, Client> _clients;
};
