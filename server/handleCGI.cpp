/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCGI.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 13:54:03 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/09 14:16:51 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"


void Server::startCGI(int clientFd, const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &body)
{
    Client &client = _clients[clientFd];
    CGI *cgi = new CGI(); // ou un CGI membre du Client si tu préfères
    if (!cgi->start(interpreter, scriptPath, method, query, body)) {
        delete cgi;
        client.outBuffer = "HTTP/1.1 500 DONT KNOW WHAT IT IS 2.0\r\n Content-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        client.responseReady = true;
        setClientPollout(clientFd);
        return; }

    client.CGIActive = true; client.CGIPid    = cgi->getPid();
    client.CGIFdOut  = cgi->getFdOut(); client.CGIFdIn   = cgi->getFdIn();
    client.CGIInput  = body; client.CGIOutput.clear();
    client.CGIStart  = std::time(NULL);
    delete cgi; // les fd et le pid sont copiés, l'objet n'est plus utile

    struct pollfd p;
    p.fd = client.CGIFdOut; p.events = POLLIN; p.revents = 0;
    _pollFds.push_back(p); _CGIToClient[client.CGIFdOut] = clientFd;
    if (client.CGIFdIn != -1) {
        struct pollfd q;
        q.fd = client.CGIFdIn; q.events = POLLOUT; q.revents = 0;
        _pollFds.push_back(q); _CGIToClient[client.CGIFdIn] = clientFd;
    }
}

void Server::handleCGIRead(std::size_t index)
{
    int CGIFd = _pollFds[index].fd;
    std::map<int,int>::iterator m = _CGIToClient.find(CGIFd);
    if (m == _CGIToClient.end()) { _pollFds[index].fd = -1; return; }
    Client &client = _clients[m->second];

    char buf[4096];
    ssize_t n = read(CGIFd, buf, sizeof(buf));
    if (n > 0) { client.CGIOutput.append(buf, n); return; }

    // EOF -> le CGI a fini d'écrire
    if (client.CGIPid > 0) { waitpid(client.CGIPid, NULL, 0); client.CGIPid = -1; }
    _CGIToClient.erase(CGIFd);
    close(CGIFd);
    client.CGIFdOut = -1;
    _pollFds[index].fd = -1;

    if (client.CGIFdIn != -1) // stdin encore ouvert -> on le ferme
    {
        _CGIToClient.erase(client.CGIFdIn);
        disablePollFdByFd(client.CGIFdIn);
        close(client.CGIFdIn);
        client.CGIFdIn = -1;
    }
    finishCGI(m->second);
}

void Server::handleCGIWrite(std::size_t index)
{
    int CGIFd = _pollFds[index].fd;
    std::map<int,int>::iterator m = _CGIToClient.find(CGIFd);
    if (m == _CGIToClient.end()) { _pollFds[index].fd = -1; return; }
    Client &client = _clients[m->second];

    if (!client.CGIInput.empty())
    {
        ssize_t n = write(CGIFd, client.CGIInput.c_str(), client.CGIInput.size());
        if (n > 0) client.CGIInput.erase(0, n);
    }
    if (client.CGIInput.empty()) // tout envoyé -> EOF pour le CGI
    {
        _CGIToClient.erase(CGIFd);
        close(CGIFd);
        client.CGIFdIn = -1;
        _pollFds[index].fd = -1;
    }
}

void Server::finishCGI(int clientFd)
{
    std::map<int,Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end()) return;
    it->second.outBuffer = CGI::buildResponse(it->second.CGIOutput);
    it->second.responseReady = true;
    it->second.CGIActive = false;
    setClientPollout(clientFd);
}

bool Server::isCGIFd(int fd) const
{
    return _CGIToClient.find(fd) != _CGIToClient.end();
}
