#include "CGI.hpp"

// Initialize the CGI process state.
CGI::CGI(const Request &req): _pid(-1), _fdIn(-1), _fdOut(-1),
    method(req.method), protocol(req.version), query(req.query),
    requestUri(req.path), body(req.body), headers(req.headers)
{
    if (!req.query.empty()) requestUri += "?" + req.query;
}
// Return the child process id.
pid_t CGI::getPid(void) const { return _pid; }
// Return the CGI stdin pipe file descriptor.
int CGI::getFdIn(void) const { return _fdIn; }
// Return the CGI stdout pipe file descriptor.
int CGI::getFdOut(void) const { return _fdOut; }