
#include "HTTP.hpp"
#include <algorithm>

// Build a directory listing page for a readable folder.
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

// Select the target location and dispatch the HTTP method.
std::string Response::build(const Request &req, const ServerConfig &config)
{
    const LocationConfig *location = matchLocation(req.path, config);
    if (!location)
        return errorResponse(404, config);

    if (location->hasRedirect)
        return makeRedirect(location->redirectCode, location->redirectURL);

    const std::vector<std::string> &methods = location->allowedMethods;
    if (!methods.empty())
    {
        bool found = false;
        for (std::size_t i = 0; i < methods.size(); ++i)
            if (methods[i] == req.method) { found = true; break; }
        if (!found)
            return errorResponse(405, config);
    }

    if (req.method == "GET")    return handleGET   (req, *location, config);
    if (req.method == "POST")   return handlePOST  (req, *location, config);
    if (req.method == "DELETE") return handleDELETE(req, *location, config);

    return errorResponse(405, config);
}