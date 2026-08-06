#include "HTTP.hpp"

// Handle a GET request using the matched location.
std::string Response::handleGET(const Request &req, const LocationConfig &location, const ServerConfig &config)
{
    std::string path = resolvePath(req.path, location);
    struct stat st;
    if (stat(path.c_str(), &st) != 0) { return errorResponse(404, config); }
    if (S_ISDIR(st.st_mode)) {
        std::string indexPath = path;
        if (lastC(indexPath) != '/') { indexPath += "/"; }
        indexPath += location.index;
        struct stat ist;
        if (!location.index.empty() && stat(indexPath.c_str(), &ist) == 0 && S_ISREG(ist.st_mode)) { path = indexPath; }
        else if (location.autoIndex) { return makeResponse(200, "text/html", buildDirectoryListing(req.path, path)); }
        else { return errorResponse(403, config); }   
    }
    bool ok = false; std::string body = readFile(path, ok);
    if (!ok) { return errorResponse(403, config); }
    return makeResponse(200, getMime(path), body);
}

// Handle a POST request and store the uploaded body.
std::string Response::handlePOST(const Request &req, const LocationConfig &location, const ServerConfig &config)
{
    if (req.body.size() > config.clientMaxBodySize) { return errorResponse(413, config); }
    if (!location.uploadEnabled || location.uploadDir.empty()) { return errorResponse(403, config); }
    std::string name = req.path.substr(req.path.find_last_of('/') + 1);
    if (name.empty()) { name = "upload"; }
    std::string dest = location.uploadDir;
    if (lastC(dest) != '/') { dest += "/"; }
    dest += name;
    int fd = open(dest.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) { return errorResponse(500, config); }
    size_t written = 0;
    while (written < req.body.size())
    {
        ssize_t chunk = write(fd, req.body.data() + written, req.body.size() - written);
        if (chunk <= 0)
        {
            close(fd);
            return errorResponse(500, config);
        }
        written += static_cast<size_t>(chunk);
    }
    close(fd);
    return makeResponse(201, "text/plain", "Created");
}

// Handle a DELETE request by removing the target file.
std::string Response::handleDELETE(const Request &req, const LocationConfig &location, const ServerConfig &config)
{
    std::string path = resolvePath(req.path, location);
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
    {
        if (location.uploadEnabled && !location.uploadDir.empty())
        {
            std::string uploadPath = location.uploadDir;
            if (lastC(uploadPath) != '/') { uploadPath += "/"; }
            uploadPath += req.path.substr(req.path.find_last_of('/') + 1);
            if (stat(uploadPath.c_str(), &st) == 0)
                path = uploadPath;
            else
                return errorResponse(404, config);
        }
        else
            return errorResponse(404, config);
    }
    if (S_ISDIR(st.st_mode)) { return errorResponse(403, config); }
    if (std::remove(path.c_str()) != 0) { return errorResponse(403, config); }
    return makeResponse(204, "text/plain", "");
}
