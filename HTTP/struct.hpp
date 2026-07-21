
#pragma once

#include <map>
#include <string>
#include <vector>

struct Request
{
    std::string method;
    std::string path;
    std::string query;
    std::string version;
    std::string body;
    std::map<std::string, std::string> headers;

    Request() {}
};