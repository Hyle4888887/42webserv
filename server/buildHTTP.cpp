/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   buildHTTP.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 14:02:13 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/10 10:58:52 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

#include <cctype>

static std::string toLowerCopy(const std::string &s)
{
	std::string out = s;
	for (std::size_t i = 0; i < out.size(); ++i)
		out[i] = static_cast<char>(std::tolower(out[i]));
	return out;
}

static std::string trimCopy(const std::string &s)
{
	std::size_t begin = 0;
	while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin])))
		++begin;

	std::size_t end = s.size();
	while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1])))
		--end;

	return s.substr(begin, end - begin);
}

/*construire la reponse HTTP*/
void	Server::buildResponse(Client &client, const std::string &rawRequest)
{
	Request req;
	req.path = "/";
	req.version = "HTTP/1.1";

	std::string::size_type lineEnd = rawRequest.find("\r\n");
	if (lineEnd != std::string::npos)
	{
		std::string requestLine = rawRequest.substr(0, lineEnd);
		std::istringstream firstLine(requestLine);
		std::string target;
		firstLine >> req.method >> target >> req.version;
		if (!target.empty())
		{
			req.path = target;
			std::string::size_type qPos = target.find('?');
			if (qPos != std::string::npos)
			{
				req.path = target.substr(0, qPos);
				req.query = target.substr(qPos + 1);
			}
		}
	}

	std::string::size_type headersStart = (lineEnd == std::string::npos) ? 0 : lineEnd + 2;
	std::string::size_type headersEnd = rawRequest.find("\r\n\r\n");
	if (headersEnd != std::string::npos && headersEnd >= headersStart)
	{
		std::size_t cursor = headersStart;
		while (cursor < headersEnd)
		{
			std::string::size_type next = rawRequest.find("\r\n", cursor);
			if (next == std::string::npos || next > headersEnd)
				break;
			std::string line = rawRequest.substr(cursor, next - cursor);
			std::string::size_type sep = line.find(':');
			if (sep != std::string::npos)
			{
				std::string key = toLowerCopy(trimCopy(line.substr(0, sep)));
				std::string value = trimCopy(line.substr(sep + 1));
				req.headers[key] = value;
			}
			cursor = next + 2;
		}
		req.body = rawRequest.substr(headersEnd + 4);
	}

	client.outBuffer = Response::build(req, _config);
}
