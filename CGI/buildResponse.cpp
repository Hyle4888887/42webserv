/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   buildResponse.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 16:30:27 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/11 14:05:49 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

static std::string toLower(const std::string &name)
{
    std::string lower = name;
    for (std::string::iterator it = lower.begin(); it != lower.end(); ++it)
    { if (*it >= 'A' && *it <= 'Z') { *it = *it + ('a' - 'A'); } }
    return lower;
}

std::string CGI::buildResponse(const std::string &cgiOut)
{
    //creation en-tete
    std::string headerBlock, cgiBody; std::string::size_type sep = cgiOut.find("\r\n\r\n");
    size_t sepLen = 4;
    if (sep == std::string::npos) { sep = cgiOut.find("\n\n"); sepLen = 2; }
    if (sep != std::string::npos) { headerBlock = cgiOut.substr(0, sep); cgiBody = cgiOut.substr(sep + sepLen); }
    else { cgiBody = cgiOut; }
    //analyse des entete
    std::string status = "200 OK", forwarded; bool hasStatus = false, hasLocation = false;
    std::istringstream hs(headerBlock); std::string line;
    while (std::getline(hs, line)) {
        if (!line.empty() && lastC(line) == '\r') { line.erase(line.size() - 1); }
        if (line.empty()) { continue; }
        
        std::string::size_type colon = line.find(':');
        if (colon == std::string::npos) { continue; } // permet d'ignorer les lignes malformées
        
        std::string name = line.substr(0, colon), value = line.substr(colon + 1);
        
        std::string::size_type vs = value.find_first_not_of(" \t");
        value = (vs == std::string::npos) ? "" : value.substr(vs);
        
        std::string lower = toLower(name);
        if (lower == "status") { status = value; hasStatus = true;}
        else if (lower == "content-lenght") { continue; }
        else { if (lower == "location") { hasLocation = true; } forwarded += name + ": " + value + "\r\n"; }
    }
    if (hasLocation && !hasStatus) { status = "302 Found"; }
    
    /* Formattage de reponse à voir avec maxime */
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n" ;
    response << forwarded;
    response << "Content-Length: " << cgiBody.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n" << cgiBody;
    return response.str();
}
