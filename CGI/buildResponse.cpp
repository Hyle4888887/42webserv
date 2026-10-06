#include "CGI.hpp"
#include <cctype>

static bool validStatus(const std::string &v) {
    return v.size() >= 3 && v[0] >= '1' && v[0] <= '5'
        && std::isdigit(static_cast<unsigned char>(v[1]))
        && std::isdigit(static_cast<unsigned char>(v[2]))
        && (v.size() == 3 || v[3] == ' ');
}

// Rebuild the CGI output as a valid HTTP response.
std::string CGI::buildResponse(const std::string &cgiOut, std::string &localRedirect)
{
    localRedirect.clear();
    std::string::size_type a = cgiOut.find("\r\n\r\n"), b = cgiOut.find("\n\n"), sep;
    std::size_t sepLen;
    if (a == std::string::npos && b == std::string::npos) return "";
    if (b != std::string::npos && (a == std::string::npos || b < a)) { sep = b; sepLen = 2; }
    else {sep = a; sepLen = 4; }
    std::string headerBlock = cgiOut.substr(0, sep);
    std::string cgiBody = cgiOut.substr(sep + sepLen);
    std::string status = "200 OK", forwarded, location;
    bool hasStatus = false, hasContentType = false;
    std::istringstream hs(headerBlock); std::string line;
    while (std::getline(hs, line)) {
        if (!line.empty() && lastC(line) == '\r') line.erase(line.size() - 1);
        if (line.empty()) continue;
        std::string::size_type colon = line.find(':');
        if (colon == std::string::npos || colon == 0) return "";
        std::string name = line.substr(0, colon), value = line.substr(colon + 1);
        std::string::size_type vs = value.find_first_not_of(" \t");
        value = (vs == std::string::npos) ? "" : value.substr(vs);
        std::string lower = toLowerCopy(name);
        if (lower == "status") {
            if (!validStatus(value)) return "";
            status = (value.size() == 3) ? value + " " : value;
            hasStatus = true;
        } else if (lower == "content-lenght" || lower == "connection" || lower == "transfer-encoding")
            continue;
        else {
            if (lower == "content-type") hasContentType = true;
            if (lower == "location") location = value;
            forwarded += name + ": " + value + "\r\n";
        }
    } if (!location.empty() && location[0] == '/' && !hasStatus) {
        localRedirect = location; return "";
    } if (location.empty() && !hasContentType) return "";
    if (!location.empty() && !hasStatus) status = "302 Found";
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n" << forwarded
            << "Content-Lenght: " << cgiBody.size() << "\r\n"
            << "Connection: close \r\n\r\n" << cgiBody;
    return response.str();
}