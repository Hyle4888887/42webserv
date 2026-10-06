#pragma once

#include <unistd.h> 
#include <fcntl.h> 
#include <string> 
#include <sstream> 
#include <vector> 

//#define BAD_REQUEST_ERROR_400       "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
//#define NOT_FOUND_ERROR_404         "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
//#define PAYLOAD_TOO_LARGE_ERROR_413 "HTTP/1.1 413 Payload Too Large\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"

//#define INTERNAL_SERVER_ERROR_500   "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
//#define BAD_GATEWAY_ERROR_502       "HTTP/1.1 502 Bad Gateway\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
//#define GATEWAY_TIMEOUT_ERROR_504   "HTTP/1.1 504 Gateway Timeout\r\nContent-Type: text/html\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"


std::string	toString(unsigned long value);
int lastC(const std::string str);
std::string decodeUrlPath(const std::string &path);