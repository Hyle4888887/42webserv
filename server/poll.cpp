#include "server.hpp"

// Mark a poll entry as inactive so it is ignored later.
void Server::disablePollFdByFd(int fd)
{
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd == fd) { _pollFds[i].fd = -1; return; }
}

// Switch a client socket to POLLOUT readiness.
void Server::setClientPollout(int clientFd)
{
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd == clientFd) { _pollFds[i].events = POLLOUT; return; }
}

// Update the poll events for a specific client.
void Server::setClientsEvents(int clientFd, short events) 
{
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd == clientFd) { _pollFds[i].events = events; return; }
}

// Remove invalid or closed poll entries to keep the array clean.
void Server::compactPollFds()
{
    std::vector<struct pollfd> kept;
    kept.reserve(_pollFds.size());
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd >= 0) kept.push_back(_pollFds[i]);
    _pollFds.swap(kept);
}