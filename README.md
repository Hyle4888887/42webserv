*This project has been created as part of the 42 curriculum by Bozil, Mbores & Mpoirier.*

# Description

Webserv is a C++98 HTTP server for the 42 curriculum. It parses an NGINX-inspired configuration file, listens on one or more interfaces and ports, and serves static content with GET, POST, and DELETE support.

The server also supports route-specific configuration, file uploads, redirects, default error pages, directory listing, and CGI execution by file extension.


# Instructions

Build the project with:

```bash
make
```

Run the server with a configuration file:

```bash
./webserv config/conf_default
```

You can also use the alternate test configuration:

```bash
./webserv config/test.conf
```

on your web browser, write: http://localhost:portnumber/


Makefile targets:

```bash
make all
make clean
make fclean
make re
```

# Resources

- RFC 7230, RFC 7231, and RFC 7232 for HTTP message formatting and semantics.
- MDN Web Docs for practical HTTP request and response behavior.
- NGINX configuration reference for server and location block structure.
- Linux man pages for `socket`, `bind`, `listen`, `accept`, `poll`, `send`, `recv`, `fork`, `execve`, `pipe`, `dup2`, `waitpid`, `getaddrinfo`, `fcntl`, and file operations.

AI was used to make the README.