#include "utils.hpp"

// Convert an unsigned integer to a string.
std::string	toString(unsigned long value)
{
	std::ostringstream	oss;
	oss << value;
	return oss.str();
}

// Return the last character of a string.
int lastC(const std::string str) { return str[str.size() - 1]; }

static int hexValue(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

std::string decodeUrlPath(const std::string &path) {
	std::string decoded;
	for (std::size_t i = 0; i < path.size(); ++i) {
		if (path[i] == '%' && i + 2 < path.size()) {
			int high = hexValue(path[i + 1]);
			int low = hexValue(path[i + 2]);
			if (high >= 0 && low >= 0) {
				decoded += static_cast<char>((high << 4) | low);
				i += 2; continue;
			}
		} decoded += path[i];
	} return decoded;
}

std::string toLowerCopy(const std::string &s)
{
    std::string out = s;
    for (std::size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return out;
}

std::string trimCopy(const std::string &s)
{
	std::size_t begin = 0;
	while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin]))) ++begin;
	std::size_t end = s.size();
	while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
	return s.substr(begin, end - begin);
}

// Join two path fragments while preserving separators.
std::string joinPath(const std::string &base, const std::string &suffix)
{
    if (base.empty()) return suffix;
    if (suffix.empty()) return base;
    if (base[base.size() - 1] == '/' && suffix[0] == '/')
        return base + suffix.substr(1);
    if (base[base.size() - 1] != '/' && suffix[0] != '/')
        return base + "/" + suffix;
    return base + suffix;
}

// Find the best matching location for a request path.
const LocationConfig *matchLocation(const std::string &path, const ServerConfig &config)
{
    const LocationConfig *best = NULL;
    std::size_t bestLen = 0;

    for (std::vector<LocationConfig>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it) {
        const std::string &prefix = it->path;
        if (prefix.empty()) continue;
        if (path.compare(0, prefix.size(), prefix) == 0
            && prefix.size() >= bestLen
            && (prefix == "/" || path.size() == prefix.size() || path[prefix.size()] == '/'))
        { best = &(*it); bestLen = prefix.size(); }
    }
    return best;
}

// Resolve a request path against the matched location root.
std::string resolvePath(const std::string &urlPath, const LocationConfig &location)
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

std::string baseName(const std::string &path)
{
    std::string::size_type slash = path.find_last_of("/\\");
    if (slash == std::string::npos)
        return path;
    return path.substr(slash + 1);
}
