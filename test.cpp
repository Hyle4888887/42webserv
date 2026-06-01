/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 13:33:16 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/01 13:51:05 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>


std::string toString(long n) { std::ostringstream oss; oss << n; return oss.str(); }

std::vector<std::string> buildEnv(const Request &req)