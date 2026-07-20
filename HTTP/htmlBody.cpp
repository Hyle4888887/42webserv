/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   htmlBody.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 15:13:21 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/11 15:14:47 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP.hpp"

std::string errorBody(const std::string code, const std::string status)
{
    std::string res;
    res += "<html><head><title>" + code + " " + status + "</title></head>";
    res += "<body style=\"text-align:center; font-family:sans-serif; margin-top:50px;\">";
    res += "<h1>" + code + " " + status + "</h1>";
    res += "<img src=\"https://http.cat/" + code + ".jpg\" ";
    res += "alt=\"" + code + " " + status + "\" ";
    res += "onerror=\"this.style.display='none';\" ";
    res += "style=\"max-width:600px; width:90%;\" />";
    res += "</body></html>";
    return res;
}