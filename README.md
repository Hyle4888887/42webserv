*This project has been created as part of the 42 curriculum by Bozil, Mbores & Mpoirier.*

# Description

Webserv is a C++98 HTTP server for the 42 curriculum. It parses an NGINX-inspired configuration file, listens on one or more interfaces and ports, and serves static content with GET, POST, and DELETE support.

The server also supports route-specific configuration, file uploads, redirects, default error pages, directory listing, and CGI execution by file extension.

# Project structure

- `server/` — Core server logic: sockets, `poll()` loop, main event loop (`server.cpp`), timeout handling, HTTP response building, and CGI dispatch (`handleCGI.cpp`, `handleOperation.cpp`).
- `HTTP/` — The `HTTP` class: request parsing and response construction/formatting (`build.cpp`, `handle.cpp`, `response.cpp`).
- `CGI/` — CGI script execution: process launching (`start.cpp`) and CGI response building (`buildResponse.cpp`).
- `parser/` — Config file parser: lexer and parser (`lexer.cpp`, `configParser.cpp`) plus the `serverConfig`/`locationConfig` structures.
- `config/` — Server configuration files (`test.conf`, `conf_default`, `conf_template`).
- `cgi-bin/` — Sample Python CGI scripts for testing (`echo.py`, `hello.py`, `redirect.py`, `notfound.py`, `lecture.py`).
- `utils/` — Shared utility functions.
- `errors/` — Default HTML error pages (404, 500).
- `uploads/` — Destination folder for files uploaded through the server.
- `www/` — Web root served by the server (`index.html`).

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