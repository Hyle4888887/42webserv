*This project has been created as part of the 42 curriculum by bozil, mbores, mpoirier.*

# Description

Webserv is a C++98 HTTP server for the 42 curriculum. It parses an NGINX-inspired configuration file, listens on one or more interfaces and ports, and serves static content with GET, POST, and DELETE support.

The server also supports route-specific configuration, file uploads, redirects, default error pages, directory listing, and CGI execution by file extension.

**Project structure:**

- `main.cpp` / `webserv.hpp` — Entry point and global header.
- `Makefile` — Build rules (`all`, `clean`, `fclean`, `re`).
- `server/` — Core server loop: socket setup (`socket.cpp`), `poll()` event loop (`poll.cpp`), client I/O handling (`handleOperation.cpp`), CGI dispatch (`handleCGI.cpp`), response assembly (`buildHTTP.cpp`) and connection timeouts (`checkTimeout.cpp`).
- `HTTP/` — HTTP request parsing and handling (`HTTP.cpp`, `handle.cpp`), response generation (`build.cpp`, `response.cpp`) and shared request/response structures (`struct.hpp`).
- `CGI/` — CGI execution: process startup with `fork`/`execve` (`start.cpp`), CGI class (`CGI.cpp`) and conversion of CGI output into an HTTP response (`buildResponse.cpp`).
- `parser/` — Config file parser: lexer and tokens (`lexer.cpp`, `token.hpp`), parser (`configParser.cpp`) and the `serverConfig` / `locationConfig` structures.
- `config/` — Configuration files: `conf_default`, `test.conf`, `multi.conf` (two sites on different ports) and `conf_template`.
- `utils/` — Shared utility functions.
- `uploads/` — Destination folder for files uploaded through the server.
- `www/` — Web root (`index.html`) and default error pages in `www/errors/` (400, 401, 403, 404, 429, 451, 500, 502, 503, 504).

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

To test two websites on different ports:

```bash
./webserv config/multi.conf
```

To open the web page on your web browser, use: http://localhost:portnumber/

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

AI was used to help draft and review this README and to use the tester.
