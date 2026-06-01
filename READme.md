***Please, do not creat branch ! - Mathilde***

**Some notes to start webserv project:**
 -  All I/O(input output) operations on sockets and pipes must go through a single poll() (or equivalent) that monitors both reading and writing simultaneously — regular disk files are exempt.
 - Every network file descriptor must be non-blocking, and no read/write may be called without prior readiness confirmation from poll(); checking errno after those calls is strictly forbidden.
 - The server must support at minimum GET, POST, and DELETE with accurate HTTP response status codes.
 - The server must be able to serve a fully static website and allow clients to upload files.
 - Default error pages must be provided when none are configured, and no request should ever hang indefinitely.
 - The server must be able to listen on multiple ports simultaneously to serve different content.
 - Inspired by NGINX, the config file defines listen pairs, error pages, max body size, and per-route rules (accepted methods, redirections, root directory, directory listing, default file, upload path, CGI execution by extension).
 - CGI(Common Gateway Interface) scripts are executed via fork/execve (the only authorized use of fork) with correct environment variables, proper unchunking of chunked requests, and EOF handling for output without content_length.

**Task distribution:**
- Person 1 — Network Core & Event Loop:
    - Set up sockets (bind, listen, accept) and manage multiple simultaneous connections.
    - Implement the main event loop using poll() / epoll() / select() / kqueue() monitoring both read and write events.
    - Ensure all network file descriptors are non-blocking and that no read/write is performed without prior poll readiness.
    - Handle client disconnections gracefully and ensure no request ever hangs indefinitely.
    - Support listening on multiple ports simultaneously.
    - Stress test the server to guarantee it remains available and never crashes.

- Person 2 — HTTP Parsing & Configuration File:
    - Parse the configuration file (listen pairs, error pages, max body size, per-route rules: methods, redirections, root, directory listing, default file, upload path).
    - Parse incoming HTTP requests (method, headers, body) and handle chunked transfer encoding by unchunking before further processing.
    - Build HTTP responses with accurate status codes and proper headers.
    - Serve a fully static website and handle file uploads from clients.
    - Provide default error pages when none are configured.

- Person 3 — CGI, Testing & README:
    - Implement CGI execution via fork/execve with correct environment variables and proper stdin/stdout piping.
    - Support at least one CGI type (php-cgi or Python) and handle output with or without content_length using EOF as the terminator.
    - Ensure the CGI runs in the correct working directory for relative path resolution.
    - Write configuration files and default demo files to showcase every feature during evaluation.
    - Test across multiple browsers, compare behavior with NGINX, and write tests in Python, Golang, or another suitable language.
    - Write the README.md (description, instructions, resources, and AI usage disclosure).

**Description:**

**Instructions:**

**Resources:**

**Additional things:**