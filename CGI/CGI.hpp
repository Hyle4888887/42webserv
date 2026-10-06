/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 12:28:33 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/04 12:28:33 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>

#include "../utils/utils.hpp"
#include "../HTTP/struct.hpp"

class CGI
{
    private:
        pid_t _pid;
        int _fdIn;
        int _fdOut;
    public:
        std::string interpreter, scriptPath, scriptName, pathInfo, pathTranslated;
        std::string method, protocol, query, requestUri, body;
        std::string serverName, serverPort, remoteAddr, remotePort;
        std::map<std::string, std::string> headers;

        //Dans le CGI.cpp
        CGI(const Request &req);
        pid_t getPid(void) const;
        int getFdIn(void) const;
        int getFdOut(void) const;

        // start et buildResponse sont chacune dans leur fichier
        bool start(void);
        static std::string buildResponse(const std::string &cgiOut, std::string &localRedirect);
        
};