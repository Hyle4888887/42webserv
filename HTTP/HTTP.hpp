/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 10:22:11 by bozil             #+#    #+#             */
/*   Updated: 2026/06/10 12:53:33 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <dirent.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <iostream>
#include "struct.hpp"
#include "../utils/utils.hpp"

class Response
{
  public:
    static std::string build(const Request &req, const ServerConfig &config);

  private:
    Response();
    
    /*HTTP*/
    static std::string handleGET   (const Request &req, const RouteConfig &route, const ServerConfig &config);
    static std::string handlePOST  (const Request &req, const RouteConfig &route, const ServerConfig &config);
    static std::string handleDELETE(const Request &req, const RouteConfig &route, const ServerConfig &config);

    /*Helpers*/
    static const RouteConfig *matchRoute(const std::string &path, const ServerConfig &config);
    static std::string        resolvePath(const std::string &urlPath, const RouteConfig &route);
    static std::string        getMime(const std::string &path);
    static std::string        readFile(const std::string &path, bool &ok);
    static std::string        buildDirectoryListing(const std::string &urlPath, const std::string &fsPath);
    static std::string        makeResponse(int code, const std::string &mime, const std::string &body);
    static std::string        makeRedirect(const std::string &location);
    static std::string        errorResponse(int code, const ServerConfig &config);
    static std::string        statusText(int code);
};
