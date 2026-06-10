/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 10:39:29 by bozil             #+#    #+#             */
/*   Updated: 2026/06/10 11:09:23 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP.hpp"

#include <dirent.h>

std::string Response::buildDirectoryListing(const std::string &urlPath, const std::string &fsPath)
{
    DIR *dir = opendir(fsPath.c_str());
    if (!dir) return "";

    std::string html;
    html  = "<html><head><title>Index of " + urlPath + "</title></head><body>";
    html += "<h1>Index of " + urlPath + "</h1><hr><pre>";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == ".") continue;
        html += "<a href=\"" + urlPath + (urlPath[urlPath.size()-1] == '/' ? "" : "/") + name + "\">" + name + "</a>\n";
    }
    closedir(dir);
    html += "</pre><hr></body></html>";
    return html;
}

std::string Response::build(const Request &req, const ServerConfig &config)
{
    const RouteConfig *route = matchRoute(req.path, config);
    if (!route)
        return errorResponse(404, config);

    const std::vector<std::string> &methods = route->allowedMethods;
    if (!methods.empty())
    {
        bool found = false;
        for (std::size_t i = 0; i < methods.size(); ++i)
            if (methods[i] == req.method) { found = true; break; }
        if (!found)
            return errorResponse(405, config);
    }

    if (req.method == "GET")    return handleGET   (req, *route, config);
    if (req.method == "POST")   return handlePOST  (req, *route, config);
    if (req.method == "DELETE") return handleDELETE(req, *route, config);

    return errorResponse(405, config);
}