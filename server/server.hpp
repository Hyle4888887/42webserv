#pragma once

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>

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
#include "../parser/config.hpp"
#include "../CGI/CGI.hpp"
#include "../utils/utils.hpp"

#ifndef POLLRDHUP
# define POLLRDHUP 0
#endif

#define CLIENT_TIMEOUT 180
#define CGI_TIMEOUT 20


extern volatile sig_atomic_t g_stop;

class Server
{
  public:
	Server();
	explicit Server(const Config &config);
	~Server();
	bool addListener(const std::string &host, int port);
	void run();
  private:
	Server(const Server &other);
	struct	Client
	{
		std::string inBuffer;
		std::string outBuffer;
		std::size_t outBufferOffset;
		int responseFileFd;
		unsigned long long responseFileRemaining;
		bool responseReady;
		time_t lastActivityTime;

		bool        CGIActive;
		pid_t       CGIPid;
		int         CGIFdIn;
		int         CGIFdOut;
		int         listenFd;
		std::string CGIInput;
		std::size_t CGIInputOffset;
		std::string CGIOutput;
		time_t      CGIStart;
		bool        requestInitialized;
		bool        requestChunked;
		bool        requestComplete;
		bool        requestNeedChunkCRLF;
		bool        requestFinalCRLFPending;
		std::size_t requestBodyCursor;
		std::size_t requestChunkRemaining;
		std::size_t requestHeaderEnd;
		std::string requestBody;
		std::string remoteAddr;
		std::string remotePort;
		Request		cgiRequest;
		int			CGIRedirects;

		Client(): outBufferOffset(0), responseFileFd(-1), responseFileRemaining(0), responseReady(false), lastActivityTime(std::time(NULL)), CGIActive(false), CGIPid(-1), CGIFdIn(-1), CGIFdOut(-1), listenFd(-1), CGIInputOffset(0), CGIStart(0), requestInitialized(false), requestChunked(false), requestComplete(false), requestNeedChunkCRLF(false), requestFinalCRLFPending(false), requestBodyCursor(0), requestChunkRemaining(0), requestHeaderEnd(0), CGIRedirects(0) {}
	};

	bool setNonBlocking(int fd);
	int ListeningSocket(const std::string &host, int port);
	bool isListener(int fd) const;

	void handleNewConnection(int listenFd);
	void handleRead(std::size_t index);
	void handleWrite(std::size_t index);
	void closeClient(std::size_t index);
	void buildResponse(Client &client, const std::string &rawRequest);
	void buildResponse(Client &client, const Request &req);
	Request parseRequest(const std::string &rawRequest) const;
	const ServerConfig *selectServerConfig(int listenFd, const Request &req) const;
	bool findCgiTarget(const Request &req, const ServerConfig &config, std::string &interpreter, std::string &scriptName, std::string &pathInfo) const;
	void checkTimeouts();
	void checkCGITimeouts();

	std::vector<int> _listenFds;
	std::vector<struct pollfd> _pollFds;
	std::map<int, Client> _clients;
	std::map<int, int> _CGIToClient;
	std::map<int, std::string> _listenerHosts;
	std::map<int, int> _listenerPorts;
	Config _config;

	bool isCGIFd(int fd) const;
	void startCGI(int clientFd, CGI &cgi);
	void handleCGIRead(std::size_t index);
	void handleCGIError(std::size_t index);
	void handleCGIWrite(std::size_t index);
	void finishCGI(int clientFd);

	void disablePollFdByFd(int fd);
	void setClientPollout(int clientFd);
	void compactPollFds();
	
	std::string cgiServerName(const Request &req, const ServerConfig &cfg, int listenFd) const;
	void dispatchRequest(int fd, const Request &req);
	void internalRedirect(int clientFd, const std::string &target);
	void setClientsEvents(int clientFd, short events);
	std::string errorFor(int code, int listenFd, const Request &req) const;
};
