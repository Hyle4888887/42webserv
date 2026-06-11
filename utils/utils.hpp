/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 14:04:54 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/11 14:04:54 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <unistd.h> 
#include <fcntl.h> 
#include <string> 
#include <sstream> 
#include <vector> 

std::string	toString(unsigned long value);
int lastC(const std::string str);