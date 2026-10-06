/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   poll.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 13:53:46 by mpoirier          #+#    #+#             */
/*   Updated: 2026/10/03 11:14:23 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

void Server::disablePollFdByFd(int fd)
{
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd == fd) { _pollFds[i].fd = -1; return; }
}

void Server::setClientPollout(int clientFd)
{
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd == clientFd) { _pollFds[i].events = POLLOUT; return; }
}

void Server::setClientsEvents(int clientFd, short events) 
{
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd == clientFd) { _pollFds[i].events = events; return; }
}

// delete useless things
void Server::compactPollFds()
{
    std::vector<struct pollfd> kept;
    kept.reserve(_pollFds.size());
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd >= 0) kept.push_back(_pollFds[i]);
    _pollFds.swap(kept);
}