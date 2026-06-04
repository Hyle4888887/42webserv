/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 16:33:49 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/04 13:19:28 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cstdlib> 
#include <cstring>
#include <cerrno>

#include "CGI/CGI.hpp"