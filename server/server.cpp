/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 12:51:56 by bozil             #+#    #+#             */
/*   Updated: 2026/06/04 16:01:39 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

/*refaire to_string car n'existe pas en C++98
converti en std::string */
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


void Server::checkCGITimeouts()
{
    time_t now = std::time(NULL);
    for (std::map<int,Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        Client &c = it->second;
        if (!c.CGIActive || std::difftime(now, c.CGIStart) < CGI_TIMEOUT) continue;

        if (c.CGIPid > 0) { kill(c.CGIPid, SIGKILL); waitpid(c.CGIPid, NULL, 0); c.CGIPid = -1; }
        if (c.CGIFdOut != -1) { _CGIToClient.erase(c.CGIFdOut); disablePollFdByFd(c.CGIFdOut); close(c.CGIFdOut); c.CGIFdOut = -1; }
        if (c.CGIFdIn  != -1) { _CGIToClient.erase(c.CGIFdIn);  disablePollFdByFd(c.CGIFdIn);  close(c.CGIFdIn);  c.CGIFdIn  = -1; }
        c.CGIActive = false;
        c.outBuffer = "HTTP/1.1 504 Gateway TimeOut\r\n Content-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        c.responseReady = true;
        setClientPollout(it->first);
    }
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
			if (errno == EINTR) // signal recu = recommencer le poll
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
 
			// Detecte la deconnexion propre plus tot que recv() == 0
			if (!isListener(fd) && (revents & POLLRDHUP))
			{
				std::cout << "[-] POLLRDHUP fd=" << fd << std::endl;
				closeClient(i);
				continue;
			}
 
			// Nouvelle connexion
			if (isListener(fd))
				handleNewConnection(fd);
			else if (isCGIFd(fd))
			{
				if (revents & POLLIN) { handleCGIRead(i); }
				else if (revents & POLLOUT) { handleCGIWrite(i); }
			}
			// Donnees a lire
			else if (revents & POLLIN)
				handleRead(i);
 
			// Pret a envoyer la reponse
			else if (revents & POLLOUT)
				handleWrite(i);
			compactPollFds();
		}
	}
}

/*gerer les nouvelles connexions*/
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
		pfd.events |= POLLRDHUP;
		pfd.revents = 0;
		_pollFds.push_back(pfd);
		_clients[clientFd] = Client();

		std::cout << "[+] Client connecte fd=" << clientFd << std::endl;
	}
}

/*gerer les donnees recues*/
void	Server::handleRead(std::size_t index)
{
	int		fd = _pollFds[index].fd;
	char	buffer[4096];

	ssize_t	n = recv(fd, buffer, sizeof(buffer), 0);
	
	if (n == 0)
	{
		std::cout << "[-] Client deconnecte fd=" << fd << std::endl;
		closeClient(index);
		return;
	}
	
	if (n < 0)
	{
		// Ne pas verifier errno apres recv (interdit par le sujet)
		// Si poll() a signale POLLIN, recv() ne devrait pas retourner EAGAIN
		std::cerr << "recv error fd=" << fd << std::endl;
		closeClient(index);
		return;
	}

	//Parsing mini (à modifier avec maxime)
	std::string rawRequest(buffer, n); std::string method, target;
	std::istringstream lineStream(rawRequest); lineStream >> method >> target;
	
	std::string path = target, query; std::string::size_type q = target.find('?');
	if (q != std::string::npos) { path = target.substr(0, q); query = target.substr(q + 1); }
	std::string body; std::string::size_type bsep = rawRequest.find("\r\n\r\n");
	if (bsep != std::string::npos) { body = rawRequest.substr(bsep + 4); }
	//end parse

	Client &client = _clients[fd];
	client.inBuffer.append(buffer, static_cast<std::size_t>(n));
	client.lastActivityTime = std::time(NULL);
	// avec CGI
	std::string interpreter;
	bool CGI = false; std::string::size_type dot = path.rfind('.');
	if (dot != std::string::npos)
	{
		std::string ext = path.substr(dot);
		if (ext == ".py")  { CGI = true; interpreter = "/usr/bin/python3"; }
		//else if (ext == ".php") { CGI = true; interpreter = "/usr/bin/php-CGI"; }
	}

	if (CGI)
	{
		std::string scriptPath = "." + path; // à remplacer par root+location de ta config
		if (access(scriptPath.c_str(), F_OK) != 0)
		{
			client.outBuffer = "HTTP/1.1 404 PAGE NOT FOUND\r\n Content-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
			client.responseReady = true;
			_pollFds[index].events = POLLOUT;
			_pollFds[index].events |= POLLRDHUP;
		}
		else if (access(scriptPath.c_str(), R_OK) != 0)
		{
			client.outBuffer = "HTTP/1.1 403 DONT KNOW WHAT IT IS\r\n Content-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
			client.responseReady = true;
			_pollFds[index].events = POLLOUT;
			_pollFds[index].events |= POLLRDHUP;
		}
		else
			startCGI(fd, interpreter, scriptPath, method, query, body);
		// si CGI lancé : PAS de POLLOUT ici, on attend la fin du CGI
	} // CGI end
	else
	{
		buildResponse(client);
		client.responseReady = true;
		_pollFds[index].events = POLLOUT;
		_pollFds[index].events |= POLLRDHUP;
	}
}

/*construire la reponse HTTP*/
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

/*envoie la reponse*/
void	Server::handleWrite(std::size_t index)
{
	int		fd = _pollFds[index].fd;
	Client	&client = _clients[fd];

	ssize_t	n = send(fd, client.outBuffer.c_str(), client.outBuffer.size(), 0);
	if (n < 0)
	{
		// Ne pas verifier errno apres send (interdit par le sujet)
		// Si poll() a signale POLLOUT, send() ne devrait pas retourner EAGAIN
		std::cerr << "send error fd=" << fd << std::endl;
		closeClient(index);
		return;
	}
 
	client.outBuffer.erase(0, static_cast<std::size_t>(n));
	client.lastActivityTime = std::time(NULL);
 
	if (client.outBuffer.empty())
	{
		std::cout << "[i] Reponse envoyee fd=" << fd << std::endl;
		closeClient(index);
	}
}

/*ferme les clients inactifs*/
void Server::checkTimeouts()
{
	time_t now = std::time(NULL);
 
	for (std::size_t i = _pollFds.size(); i-- > 0;)
	{
		int fd = _pollFds[i].fd;
 
		if (isListener(fd))
			continue;
 
		std::map<int, Client>::iterator it = _clients.find(fd);
		if (it == _clients.end())
			continue;
 
		double elapsed = std::difftime(now, it->second.lastActivityTime);
		if (elapsed > CLIENT_TIMEOUT)
		{
			std::cout << "[!] Timeout client fd=" << fd
					  << " (inactif " << (int)elapsed << "s)" << std::endl;
			closeClient(i);
		}
	}
}

/*supprime un client -- A ete modifie */
void	Server::closeClient(std::size_t index)
{
	int	fd = _pollFds[index].fd;

	// 1. si le client avait un CGI en cours, on nettoie le CGI d'abord
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

	// 2. fermeture du client
	close(fd);
	_clients.erase(fd);
	_pollFds[index].fd = -1;   // <-- au lieu de _pollFds.erase(...)
}

// Rajouter
bool Server::isCGIFd(int fd) const
{
    return _CGIToClient.find(fd) != _CGIToClient.end();
}

void Server::startCGI(int clientFd, const std::string &interpreter,
                      const std::string &scriptPath, const std::string &method,
                      const std::string &query, const std::string &body)
{
    Client &client = _clients[clientFd];

    CGI *cgi = new CGI(); // ou un CGI membre du Client si tu préfères
    if (!cgi->start(interpreter, scriptPath, method, query, body))
    {
        delete cgi;
        client.outBuffer = "HTTP/1.1 500 DONT KNOW WHAT IT IS 2.0\r\n Content-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        client.responseReady = true;
        setClientPollout(clientFd);
        return;
    }

    client.CGIActive = true;
    client.CGIPid    = cgi->getPid();
    client.CGIFdOut  = cgi->getFdOut();
    client.CGIFdIn   = cgi->getFdIn();
    client.CGIInput  = body;
    client.CGIOutput.clear();
    client.CGIStart  = std::time(NULL);
    delete cgi; // les fd et le pid sont copiés, l'objet n'est plus utile

    struct pollfd p;
    p.fd = client.CGIFdOut; p.events = POLLIN; p.revents = 0;
    _pollFds.push_back(p);
    _CGIToClient[client.CGIFdOut] = clientFd;

    if (client.CGIFdIn != -1)
    {
        struct pollfd q;
        q.fd = client.CGIFdIn; q.events = POLLOUT; q.revents = 0;
        _pollFds.push_back(q);
        _CGIToClient[client.CGIFdIn] = clientFd;
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

void Server::compactPollFds()
{
    std::vector<struct pollfd> kept;
    kept.reserve(_pollFds.size());
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
        if (_pollFds[i].fd >= 0) kept.push_back(_pollFds[i]);
    _pollFds.swap(kept);
}