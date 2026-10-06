#pragma once

#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <csignal>
#include <cstdlib> 
#include <cstring>
#include <cerrno>

#include "server/server.hpp"
#include "utils/utils.hpp"
#include "CGI/CGI.hpp"
