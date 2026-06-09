/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleOperation.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 13:58:21 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/09 14:00:05 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

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
