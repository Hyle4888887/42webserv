/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 10:21:45 by bozil             #+#    #+#             */
/*   Updated: 2026/06/11 15:02:49 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP.hpp"

std::string Response::getMime(const std::string &path)
{
    std::string::size_type dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(dot);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css")  return "text/css";
    if (ext == ".js")   return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".png")  return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif")  return "image/gif";
    if (ext == ".ico")  return "image/x-icon";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".txt")  return "text/plain";
    if (ext == ".pdf")  return "application/pdf";
    return "application/octet-stream";
}

std::string Response::readFile(const std::string &path, bool &ok)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f.is_open()) { ok = false; return ""; }
    std::ostringstream oss;
    oss << f.rdbuf();
    if (f.bad()) { ok = false; return ""; }
    ok = true;
    return oss.str();
}

static std::string errorBody(const std::string code, const std::string status)
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

std::string Response::errorResponse(int code, const ServerConfig &config)
{
    std::map<int, std::string>::const_iterator it = config.errorPages.find(code);
    if (it != config.errorPages.end())
    {
        bool ok;
        std::string body = readFile(it->second, ok);
        if (ok) return makeResponse(code, "text/html", body);
    }
    std::string body = errorBody(toString(code), statusText(code));
    return makeResponse(code, "text/html", body);
}

const RouteConfig *Response::matchRoute(const std::string &path, const ServerConfig &config)
{
    const RouteConfig *best = NULL;
    std::size_t        bestLen = 0;

    for (std::map<std::string, RouteConfig>::const_iterator it = config.routes.begin();
         it != config.routes.end(); ++it)
    {
        const std::string &prefix = it->first;
        // La route doit etre un prefixe du chemin
        if (path.substr(0, prefix.size()) == prefix
            && prefix.size() >= bestLen)
        {
            best    = &it->second;
            bestLen = prefix.size();
        }
    }
    return best;
}

std::string Response::resolvePath(const std::string &urlPath, const RouteConfig &route)
{
    std::string::size_type pos = 0;
    while ((pos = urlPath.find("..", pos)) != std::string::npos)
    {
        bool before = (pos == 0 || urlPath[pos - 1] == '/');
        bool after  = (pos + 2 == urlPath.size() || urlPath[pos + 2] == '/');
        if (before && after)
            return "";
        pos += 2;
    }

    std::string fs = route.root;
    if (!fs.empty() && lastC(fs) == '/')
        fs.erase(fs.size() - 1);
    fs += urlPath;
    return fs;
}

