***This project has been created as part of the 42 curriculum by Bozil, Mbores & Mpoirier***

**Some notes to start webserv project:**
 -  All I/O(input output) operations on sockets and pipes must go through a single poll() (or equivalent) that monitors both reading and writing simultaneously — regular disk files are exempt.
 - Every network file descriptor must be non-blocking, and no read/write may be called without prior readiness confirmation from poll(); checking errno after those calls is strictly forbidden.
 - The server must support at minimum GET, POST, and DELETE with accurate HTTP response status codes.
 - The server must be able to serve a fully static website and allow clients to upload files.
 - Default error pages must be provided when none are configured, and no request should ever hang indefinitely.
 - The server must be able to listen on multiple ports simultaneously to serve different content.
 - Inspired by NGINX, the config file defines listen pairs, error pages, max body size, and per-route rules (accepted methods, redirections, root directory, directory listing, default file, upload path, CGI(Common Gateway Interface) execution by extension).
 - CGI scripts are executed via fork/execve (the only authorized use of fork) with correct environment variables, proper unchunking of chunked requests, and EOF handling for output without content_length.

**Task distribution:**
- Bozil — Network Core & Event Loop:
    - Set up sockets (bind, listen, accept) and manage multiple simultaneous connections.
    - Implement the main event loop using poll() / epoll() / select() / kqueue() monitoring both read and write events.
    - Ensure all network file descriptors are non-blocking and that no read/write is performed without prior poll readiness.
    - Handle client disconnections gracefully and ensure no request ever hangs indefinitely.
    - Support listening on multiple ports simultaneously.
    - Stress test the server to guarantee it remains available and never crashes.

- Mbores — HTTP Parsing & Configuration File:
    - Parse the configuration file (listen pairs, error pages, max body size, per-route rules: methods, redirections, root, directory listing, default file, upload path).
    - Parse incoming HTTP requests (method, headers, body) and handle chunked transfer encoding by unchunking before further processing.
    - Build HTTP responses with accurate status codes and proper headers.
    - Serve a fully static website and handle file uploads from clients.
    - Provide default error pages when none are configured.

- Mpoirier — CGI(Common Gateway Interface), Testing & README:
    - Implement CGI execution via fork/execve with correct environment variables and proper stdin/stdout piping.
    - Support at least one CGI type (php-cgi or Python) and handle output with or without content_length using EOF as the terminator.
    - Ensure the CGI runs in the correct working directory for relative path resolution.
    - Write configuration files and default demo files to showcase every feature during evaluation.
    - Test across multiple browsers, compare behavior with NGINX, and write tests in Python, Golang, or another suitable language.
    - Write the README.md (description, instructions, resources, and AI usage disclosure).

**Description:**
- The Webserv project involves designing and implementing, as a team and in C++98, a complete HTTP server — non-blocking network management with a single poll(), request parsing, NGINX-style configuration, GET/POST/DELETE methods, and execution of CGI scripts — while respecting strict constraints of robustness, performance and compliance with the HTTP protocol.
- The goal of this project is to deepen our understanding of the HTTP protocol and low-level network programming by implementing core web server functionality without relying on any external libraries. The server supports the GET, POST, and DELETE methods, serves static websites, allows file uploads, and can execute CGI scripts (such as Python). It is configured via an NGINX-inspired configuration file, which defines listening ports, routes, error pages, and per-route rules such as allowed methods, redirections, and directory listing.
- The project emphasizes reliability and performance: it must remain non-blocking at all times, never crash under any circumstances, and stay compatible with standard web browsers and tools like curl and telnet.

**Instructions:**
- make
- ./webserv
- open new terminal
- telnet localhost 8080
- GET / HTTP/1.1 Host: localhost
- curl http://localhost:8080
- curl http://localhost:8080/redirection

**Resources:**
- cmd:man
- ibm.com
- cplusplus.com
- https://contabo.com/blog/http-response-codes-server-statuses/?utm_source=google&utm_medium=cpc&utm_campaign=brand-pmax-global&utm_term=&utm_content=&gad_source=1&gad_campaignid=23237090875&gbraid=0AAAAAD_Qy-fg_Km4x4kTjqANuto9M8HXt&gclid=CjwKCAjwuanRBhBSEiwAY5y6V6aK9XyHYQwTdtUixaY-O5vlLe97PXhMLGKYXuAfpI-5Ss6uo5XO2RoC4DQQAvD_BwE#418-im-a-teapot-45

**Additional things:**
-   Use Ai to :
    - Verify some test i did before starting the project.
    - Start somwhere else with a guide without ai coding for me.
    - Verify any bads things in the code even if it compile.
