/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 10:34:11 by bozil             #+#    #+#             */
/*   Updated: 2026/06/11 15:19:15 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP.hpp"

// sert a resoudre le path URL en path systeme de fichier en fonction de la config de la route
std::string Response::handleGET(const Request &req, const RouteConfig &route, const ServerConfig &config)
{
    std::string path = resolvePath(req.path, route);
    struct stat st;
    if (stat(path.c_str(), &st) != 0) { return errorResponse(404, config); }
    if (S_ISDIR(st.st_mode)) {
        std::string indexPath = path;
        if (lastC(indexPath) != '/') { indexPath += "/"; }
        indexPath += route.index;
        struct stat ist;
        if (!route.index.empty() && stat(indexPath.c_str(), &ist) == 0 && S_ISREG(ist.st_mode)) { path = indexPath; }
        else if (route.dirListing) { return makeResponse(200, "text/html", buildDirectoryListing(req.path, path)); }
        else { return errorResponse(403, config); }   
    }
    bool ok = false; std::string body = readFile(path, ok);
    if (!ok) { return errorResponse(403, config); }
    return makeResponse(200, getMime(path), body);
}

// sert a gerer les requetes POST (upload de fichier)
std::string Response::handlePOST(const Request &req, const RouteConfig &route, const ServerConfig &config)
{
    if (req.body.size() > config.maxBodySize) { return errorResponse(413, config); }
    if (route.uploadPath.empty()) { return errorResponse(403, config); }
    std::string name = req.path.substr(req.path.find_last_of('/') + 1);
    if (name.empty()) { name = "upload"; }
    std::string dest = route.uploadPath;
    if (lastC(dest) != '/') { dest += "/"; }
    dest += name;
    int fd = open(dest.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) { return errorResponse(500, config); }
    if (!req.body.empty()) { write(fd, req.body.data(), req.body.size()); }
    close(fd);
    return makeResponse(201, "text/plain", "Created");
}

// sert a gerer les requetes DELETE (suppression de fichier)
std::string Response::handleDELETE(const Request &req, const RouteConfig &route, const ServerConfig &config)
{
    std::string path = resolvePath(req.path, route);
    struct stat st;
    if (stat(path.c_str(), &st) != 0) { return errorResponse(404, config); }
    if (S_ISDIR(st.st_mode)) { return errorResponse(403, config); }
    if (std::remove(path.c_str()) != 0) { return errorResponse(403, config); }
    return makeResponse(204, "text/plain", "No Content");
}
