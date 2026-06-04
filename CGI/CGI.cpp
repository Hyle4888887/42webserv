/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 12:44:50 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/04 12:44:50 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(void): _pid(-1), _fdIn(-1), _fdOut(-1) {}
pid_t CGI::getPid(void) const { return _pid; }
int CGI::getFdIn(void) const { return _fdIn; }
int CGI::getFdOut(void) const { return _fdOut; }