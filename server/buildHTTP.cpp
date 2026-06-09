/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   buildHTTP.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 14:02:13 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/09 14:02:36 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

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
