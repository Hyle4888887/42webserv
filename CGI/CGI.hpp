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
#include <cstdlib>

class CGI
{
    private:
        pid_t _pid;
        int _fdIn;
        int _fdOut;
    public:
        //Dans le CGI.cpp
        CGI(void);
        pid_t getpid(void) const;
        int getFdIn(void) const;
        int getFdOut(void) const;

        // start et buildResponse sont chacune dans leur fichier
        bool start(const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::strin,g &query, const std::string &body);
        static std::string buildResponse(const std::string &cgiOut) const;
        
}

//std::string executeCGI(const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &body);
//std::string buildCGIResponse(const std::string &cgiOut);