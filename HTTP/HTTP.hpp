
#pragma once

#include <iostream>
#include <fstream> 
#include <sstream>

#include <cstring>
#include <cstdio>
#include <ctime>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#include "struct.hpp"
#include "../utils/utils.hpp"
#include "../parser/locationConfig.hpp"
#include "../parser/serverConfig.hpp"

class Response
{
  public:
    static std::string build(const Request &req, const ServerConfig &config);

  private:
    Response();
    
    /*HTTP*/
    static std::string handleGET   (const Request &req, const LocationConfig &location, const ServerConfig &config);
    static std::string handlePOST  (const Request &req, const LocationConfig &location, const ServerConfig &config);
    static std::string handleDELETE(const Request &req, const LocationConfig &location, const ServerConfig &config);

    /*Helpers*/
    static const LocationConfig *matchLocation(const std::string &path, const ServerConfig &config);
    static std::string        resolvePath(const std::string &urlPath, const LocationConfig &location);
    static std::string        getMime(const std::string &path);
    static std::string        readFile(const std::string &path, bool &ok);
    static std::string        buildDirectoryListing(const std::string &urlPath, const std::string &fsPath);
    static std::string        makeResponse(int code, const std::string &mime, const std::string &body);
    static std::string        makeRedirect(int code, const std::string &location);
    static std::string        errorResponse(int code, const ServerConfig &config);
    static std::string        statusText(int code);
};
