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

static int hexValue(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static std::string decodeUrlPath(const std::string &path)
{
    std::string decoded;
    for (std::size_t i = 0; i < path.size(); ++i)
    {
        if (path[i] == '%' && i + 2 < path.size())
        {
            int high = hexValue(path[i + 1]);
            int low = hexValue(path[i + 2]);
            if (high >= 0 && low >= 0)
            {
                decoded += static_cast<char>((high << 4) | low);
                i += 2;
                continue;
            }
        }
        decoded += path[i];
    }
    return decoded;
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
    if (ext == ".txt")  return "text/plain";
    if (ext == ".csv")  return "text/csv";
    if (ext == ".xml")  return "application/xml";
    if (ext == ".pdf")  return "application/pdf";

    // Images
    if (ext == ".png")  return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif")  return "image/gif";
    if (ext == ".ico")  return "image/x-icon";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".webp") return "image/webp";
    if (ext == ".bmp")  return "image/bmp";
    if (ext == ".tiff" || ext == ".tif") return "image/tiff";

    // Video
    if (ext == ".mp4")  return "video/mp4";
    if (ext == ".webm") return "video/webm";
    if (ext == ".ogv")  return "video/ogg";
    if (ext == ".mov")  return "video/quicktime";
    if (ext == ".avi")  return "video/x-msvideo";
    if (ext == ".mkv")  return "video/x-matroska";

    // Audio
    if (ext == ".mp3")  return "audio/mpeg";
    if (ext == ".wav")  return "audio/wav";
    if (ext == ".ogg")  return "audio/ogg";
    if (ext == ".flac") return "audio/flac";
    if (ext == ".m4a")  return "audio/mp4";

    // Documents / archives
    if (ext == ".doc")  return "application/msword";
    if (ext == ".docx") return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if (ext == ".zip")  return "application/zip";
    if (ext == ".gz")   return "application/gzip";
    if (ext == ".tar")  return "application/x-tar";

    // Fonts
    if (ext == ".woff")  return "font/woff";
    if (ext == ".woff2") return "font/woff2";
    if (ext == ".ttf")   return "font/ttf";

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
    std::string decodedUrlPath = decodeUrlPath(urlPath);
    std::string::size_type pos = 0;
    while ((pos = decodedUrlPath.find("..", pos)) != std::string::npos)
    {
        bool before = (pos == 0 || decodedUrlPath[pos - 1] == '/');
        bool after  = (pos + 2 == decodedUrlPath.size() || decodedUrlPath[pos + 2] == '/');
        if (before && after)
            return "";
        pos += 2;
    }

    std::string fs = location.root;
    if (!fs.empty() && lastC(fs) == '/')
        fs.erase(fs.size() - 1);
    std::string suffix = decodedUrlPath;
    if (decodedUrlPath.compare(0, location.path.size(), location.path) == 0)
        suffix = decodedUrlPath.substr(location.path.size());
    fs = joinPath(fs, suffix);
    return fs;
}

bool Response::prepareDownload(const Request &req, const ServerConfig &config,
                               int &fileFd, unsigned long long &fileSize,
                               std::string &headers)
{
    const LocationConfig *location = matchLocation(req.path, config);
    if (!location || location->path == "/" || !location->uploadEnabled || location->uploadDir.empty())
        return false;

    std::string path = resolvePath(req.path, *location);
    struct stat st;
    if (path.empty() || stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
        return false;

    fileFd = open(path.c_str(), O_RDONLY);
    if (fileFd < 0)
        return false;

    fileSize = static_cast<unsigned long long>(st.st_size);
    std::string fileName = path.substr(path.find_last_of("/\\") + 1);
    headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Type: " + getMime(path) + "\r\n";
    headers += "Content-Disposition: attachment; filename=\"" + fileName + "\"\r\n";
    headers += "Content-Length: " + toString(static_cast<long>(fileSize)) + "\r\n";
    headers += "Connection: close\r\n\r\n";
    return true;
}