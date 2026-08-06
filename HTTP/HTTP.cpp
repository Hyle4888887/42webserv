
#include "HTTP.hpp"

// Join two path fragments while preserving separators.
static std::string joinPath(const std::string &base, const std::string &suffix)
{
    if (base.empty())
        return suffix;
    if (suffix.empty())
        return base;
    if (base[base.size() - 1] == '/' && suffix[0] == '/')
        return base + suffix.substr(1);
    if (base[base.size() - 1] != '/' && suffix[0] != '/')
        return base + "/" + suffix;
    return base + suffix;
}

// Detect the MIME type from a file extension.
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

// Read a file fully into memory.
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

// Build a fallback HTML error page.
static std::string errorBody(const std::string code, const std::string status)
{
    std::string res;
    res += "<html><head><title>" + code + " " + status + "</title></head>";
    res += "<body style=\"text-align:center; font-family:sans-serif; margin-top:50px;\">";
    res += "<h1>" + code + " " + status + "</h1>";
    res += "<p>The server could not complete the request.</p>";
    res += "</body></html>";
    return res;
}

// Build an error response using a configured page when available.
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

// Find the best matching location for a request path.
const LocationConfig *Response::matchLocation(const std::string &path, const ServerConfig &config)
{
    const LocationConfig *best = NULL;
    std::size_t bestLen = 0;

    for (std::vector<LocationConfig>::const_iterator it = config.locations.begin();
         it != config.locations.end(); ++it)
    {
        const std::string &prefix = it->path;
        if (prefix.empty())
            continue;
        if (path.compare(0, prefix.size(), prefix) == 0
            && prefix.size() >= bestLen
            && (prefix == "/" || path.size() == prefix.size() || path[prefix.size()] == '/'))
        {
            best = &(*it);
            bestLen = prefix.size();
        }
    }
    return best;
}

// Resolve a request path against the matched location root.
std::string Response::resolvePath(const std::string &urlPath, const LocationConfig &location)
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

    std::string fs = location.root;
    if (!fs.empty() && lastC(fs) == '/')
        fs.erase(fs.size() - 1);
    std::string suffix = urlPath;
    if (urlPath.compare(0, location.path.size(), location.path) == 0)
        suffix = urlPath.substr(location.path.size());
    fs = joinPath(fs, suffix);
    return fs;
}

