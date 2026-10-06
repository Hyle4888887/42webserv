#include "HTTP.hpp"

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

bool Response::prepareDownload(const Request &req, const ServerConfig &config,
                               int &fileFd, unsigned long long &fileSize,
                               std::string &headers)
{
    const LocationConfig *location = matchLocation(req.path, config);
    if (!location || location->path == "/" || !location->uploadEnabled || location->uploadDir.empty())
        return false;

    std::string path = resolvePath(req.path, *location);
    struct stat st;
    if (path.empty() || stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) return false;
    fileFd = open(path.c_str(), O_RDONLY);
    if (fileFd < 0) return false;
    fileSize = static_cast<unsigned long long>(st.st_size);
    std::string fileName = baseName(path);
    headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Type: " + getMime(path) + "\r\n";
    headers += "Content-Disposition: attachment; filename=\"" + fileName + "\"\r\n";
    headers += "Content-Length: " + toString(static_cast<long>(fileSize)) + "\r\n";
    headers += "Connection: close\r\n\r\n";
    return true;
}