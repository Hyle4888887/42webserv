/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   struct.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 10:42:05 by bozil             #+#    #+#             */
/*   Updated: 2026/06/10 11:09:45 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <vector>

struct RouteConfig
{
    std::string              root;
    std::vector<std::string> allowedMethods;
    std::string              index;
    std::string              redirect;
    std::string              uploadPath;
    bool                     dirListing;

    RouteConfig() : dirListing(false) {}
};

struct ServerConfig
{
    int                          port;
    std::string                  host;
    std::size_t                  maxBodySize;
    std::map<int, std::string>   errorPages;
    std::map<std::string, RouteConfig> routes;

    ServerConfig() : port(8080), host("0.0.0.0"), maxBodySize(1024 * 1024) {}
};

struct Request
{
    std::string method;
    std::string path;
    std::string query;
    std::string version;
    std::string body;
    std::map<std::string, std::string> headers;

    Request() {}
};