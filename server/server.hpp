/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 12:52:06 by bozil             #+#    #+#             */
/*   Updated: 2026/06/10 11:06:45 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include <ctime>
#include <csignal>
#include <cstddef>
#include <cerrno>
#include <cstring>

#include "../HTTP/HTTP.hpp"
#include "../CGI/CGI.hpp"
#include "../utils/utils.hpp"

#ifndef POLLRDHUP
# define POLLRDHUP 0
#endif

#define CLIENT_TIMEOUT 20
#define CGI_TIMEOUT 3

class Server
{
  public:
	Server();
	~Server();
	bool addListener(int port);
	void run();

  private:
	Server(const Server &other);
	struct	Client
	{
		std::string inBuffer;
		std::string outBuffer;
		bool responseReady;
		time_t lastActivityTime;

		bool        CGIActive;
		pid_t       CGIPid;
		int         CGIFdIn;
		int         CGIFdOut;
		std::string CGIInput;
		std::string CGIOutput;
		time_t      CGIStart;

		Client(): responseReady(false), lastActivityTime(std::time(NULL)), CGIActive(false), CGIPid(-1), CGIFdIn(-1), CGIFdOut(-1), CGIStart(0) {}
	};

	bool setNonBlocking(int fd);
	int ListeningSocket(int port);
	bool isListener(int fd) const;

	void handleNewConnection(int listenFd);
	void handleRead(std::size_t index);
	void handleWrite(std::size_t index);
	void closeClient(std::size_t index);
	void buildResponse(Client &client, const std::string &rawRequest);
	void checkTimeouts();
	void checkCGITimeouts();

	std::vector<int> _listenFds;
	std::vector<struct pollfd> _pollFds;
	std::map<int, Client> _clients;
	std::map<int, int> _CGIToClient;
	ServerConfig _config;

	bool isCGIFd(int fd) const;
	void startCGI(int clientFd, const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &body);
	void handleCGIRead(std::size_t index);
	void handleCGIWrite(std::size_t index);
	void finishCGI(int clientFd);

	void disablePollFdByFd(int fd);
	void setClientPollout(int clientFd);
	void compactPollFds();
};
