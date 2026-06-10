/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 10:45:23 by bozil             #+#    #+#             */
/*   Updated: 2026/06/10 12:50:55 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP.hpp"

std::string Response::statusText(int code)
{
    switch (code)
    {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 413: return "Content Too Large";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

std::string Response::makeResponse(int code, const std::string &mime, const std::string &body)
{
    std::string r;
    r  = "HTTP/1.1 " + toString(code) + " " + statusText(code) + "\r\n";
    r += "Content-Type: "   + mime          + "\r\n";
    r += "Content-Length: " + toString((long)body.size()) + "\r\n";
    r += "Connection: close\r\n";
    r += "\r\n";
    r += body;
    return r;
}

std::string Response::makeRedirect(const std::string &location)
{
    std::string r;
    r  = "HTTP/1.1 301 Moved Permanently\r\n";
    r += "Location: " + location + "\r\n";
    r += "Content-Length: 0\r\n";
    r += "Connection: close\r\n";
    r += "\r\n";
    return r;
}