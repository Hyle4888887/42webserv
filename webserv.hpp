/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 16:33:49 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/01 16:52:44 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <cstdlib> 

std::string executeCGI(const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &body);
std::string buildCGIResponse(const std::string &cgiOut);