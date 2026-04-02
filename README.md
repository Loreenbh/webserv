# Webserv (42) – Custom HTTP Web Server

Implementation of a simplified HTTP web server in C as part of the 42 curriculum.  
The project focuses on **network programming, HTTP protocol handling, and server-side logic**.

## Skills Demonstrated

- TCP/IP networking and socket programming
- HTTP request parsing and response handling
- Multi-client handling with `poll()`
- File serving and MIME types
- Error handling (400, 404, 500 HTTP errors)
- Basic server-side scripting support (CGI)

## Project Overview

Webserv is a custom HTTP server that supports:

1. Handling multiple simultaneous clients
2. Parsing HTTP requests (`GET`, `POST`)
3. Serving static files with correct MIME types
4. Returning proper HTTP status codes for errors
5. Executing CGI scripts for dynamic content
6. Logging requests (optional)

## Getting Started
