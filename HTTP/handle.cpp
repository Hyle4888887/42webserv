#include "HTTP.hpp"

#include <cctype>

static std::string toLowerCopy(const std::string &s)
{
    std::string out = s;
    for (std::size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return out;
}

static std::string trimCopy(const std::string &s)
{
    std::string::size_type begin = 0;
    while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin])))
        ++begin;

    std::string::size_type end = s.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;

    return s.substr(begin, end - begin);
}

static std::string stripQuotes(const std::string &s)
{
    if (s.size() >= 2 && ((s[0] == '"' && s[s.size() - 1] == '"') || (s[0] == '\'' && s[s.size() - 1] == '\'')))
        return s.substr(1, s.size() - 2);
    return s;
}

static std::string baseName(const std::string &path)
{
    std::string::size_type slash = path.find_last_of("/\\");
    if (slash == std::string::npos)
        return path;
    return path.substr(slash + 1);
}

static std::string getHeaderValue(const Request &req, const std::string &name)
{
    std::map<std::string, std::string>::const_iterator it = req.headers.find(name);
    if (it == req.headers.end())
        return "";
    return it->second;
}

static bool extractMultipartUpload(const Request &req, std::string &filename, std::string &content)
{
    std::string contentType = getHeaderValue(req, "content-type");
    std::string lowerContentType = toLowerCopy(contentType);
    if (lowerContentType.find("multipart/form-data") == std::string::npos)
        return false;

    std::string::size_type boundaryPos = lowerContentType.find("boundary=");
    if (boundaryPos == std::string::npos)
        return false;

    std::string boundary = contentType.substr(boundaryPos + 9);
    std::string::size_type separator = boundary.find(';');
    if (separator != std::string::npos)
        boundary = boundary.substr(0, separator);
    boundary = stripQuotes(trimCopy(boundary));
    if (boundary.empty())
        return false;

    const std::string marker = "--" + boundary;
    std::string::size_type filenamePos = req.body.find("filename=");
    if (filenamePos == std::string::npos)
        return false;

    filenamePos += 9;
    if (filenamePos >= req.body.size())
        return false;

    std::string::size_type filenameEnd = req.body.find('"', filenamePos);
    if (req.body[filenamePos] == '"')
    {
        ++filenamePos;
        filenameEnd = req.body.find('"', filenamePos);
    }
    else
    {
        std::string::size_type semicolon = req.body.find(';', filenamePos);
        std::string::size_type cr = req.body.find('\r', filenamePos);
        std::string::size_type lf = req.body.find('\n', filenamePos);
        filenameEnd = req.body.size();
        if (semicolon != std::string::npos && semicolon < filenameEnd)
            filenameEnd = semicolon;
        if (cr != std::string::npos && cr < filenameEnd)
            filenameEnd = cr;
        if (lf != std::string::npos && lf < filenameEnd)
            filenameEnd = lf;
    }

    if (filenameEnd == std::string::npos || filenameEnd <= filenamePos)
        return false;

    filename = baseName(stripQuotes(trimCopy(req.body.substr(filenamePos, filenameEnd - filenamePos))));
    if (filename.empty())
        return false;

    std::string::size_type dataStart = req.body.find("\r\n\r\n", filenameEnd);
    std::size_t headerSepLen = 4;
    if (dataStart == std::string::npos)
    {
        dataStart = req.body.find("\n\n", filenameEnd);
        headerSepLen = 2;
    }
    if (dataStart == std::string::npos)
        return false;
    dataStart += headerSepLen;

    std::string::size_type dataEnd = req.body.find("\r\n" + marker, dataStart);
    if (dataEnd == std::string::npos)
        dataEnd = req.body.find("\n" + marker, dataStart);
    if (dataEnd == std::string::npos)
        dataEnd = req.body.find(marker + "--", dataStart);
    if (dataEnd == std::string::npos)
        return false;

    if (dataEnd >= 2 && req.body.compare(dataEnd - 2, 2, "\r\n") == 0)
        content = req.body.substr(dataStart, dataEnd - dataStart - 2);
    else if (dataEnd >= 1 && req.body[dataEnd - 1] == '\n')
        content = req.body.substr(dataStart, dataEnd - dataStart - 1);
    else
        content = req.body.substr(dataStart, dataEnd - dataStart);

    return true;
}

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
    std::string payload = req.body;

    std::string multipartName;
    std::string multipartBody;
    if (extractMultipartUpload(req, multipartName, multipartBody))
    {
        if (!multipartName.empty())
            name = multipartName;
        payload = multipartBody;
    }

    std::string dest = location.uploadDir;
    if (lastC(dest) != '/') { dest += "/"; }
    dest += name;
    int fd = open(dest.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        return errorResponse(500, config);
    size_t written = 0;
    while (written < payload.size())
    {
        ssize_t chunk = write(fd, payload.data() + written, payload.size() - written);
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
