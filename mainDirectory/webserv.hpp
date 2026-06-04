/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 16:33:49 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/04 16:04:36 by mpoirier         ###   ########.fr       */
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

#include "../server/server.hpp"