#include "CGI.hpp"

// Initialize the CGI process state.
CGI::CGI(void): _pid(-1), _fdIn(-1), _fdOut(-1) {}
// Return the child process id.
pid_t CGI::getPid(void) const { return _pid; }
// Return the CGI stdin pipe file descriptor.
int CGI::getFdIn(void) const { return _fdIn; }
// Return the CGI stdout pipe file descriptor.
int CGI::getFdOut(void) const { return _fdOut; }