ss -ltnp | grep 8080

┌────────────────────────────────┐
│            main()              │
└──────────────┬─────────────────┘
               │
               ▼
┌────────────────────────────────┐
│ Config                         │
│ - Load / hardcode configuration│
│ - Return vector<Server>        │
│   e.g., [("127.0.0.1", 8080)]  │
└──────────────┬─────────────────┘
               │
               ▼
┌────────────────────────────────┐
│ Server                         │
│ - Store one listening IP/Port  │
│ - getIp(), getPort()           │
└──────────────┬─────────────────┘
               │ (Config gives Server list)
               ▼
┌────────────────────────────────┐
│ Socket                         │
│ - Loop over Server list:       │
│   socket() → bind(IP, Port)    │
│   → listen()                   │
│ - Register listening fds in    │
│   epoll                        │
│ - socketMatch(): check if fd   │
│   is a listening socket        │
└──────────────┬─────────────────┘
               │
               ▼
┌────────────────────────────────┐
│ epoll Event Loop               │
│ - Wait for events (epoll_wait) │
│ - If listening fd + EPOLLIN:   │
│     accept() new connection    │
│     register client fd in epoll│
│ - If client fd + EPOLLIN:      │
│     recv() HTTP request        │
│     send() HTTP response       │
│     close() connection         │
└────────────────────────────────┘


Create your own HTTP server

- Accept HTTP requests (GET, POST, DELETE)

- Respond correctly with status codes and content 200->ok 404->Not Found etc..

- Serve static websites (HTML, CSS, JS files)

- Handle file uploads

- Support configuration files (similar to NGINX style)

- Work with a web browser

- Be fully non-blocking using poll() (or select(), epoll(), or kqueue())

- Handle multiple connections simultaneously (multiplexing)

- Support CGI (e.g., running a PHP or Python script)

- Stress-test ready (it must never crash or hang)


1. Core Developer (Network + IO)		

Set up sockets

Handle connections using poll()

Manage client connections (fd, buffer)
| socket, poll, accept, recv, send, non-blocking, fd |

2. HTTP Protocol Developer

Parse HTTP requests (method, headers, body)

Generate correct HTTP responses

Implement GET, POST, DELETE logic
| HTTP status codes, RequestParser, ResponseBuilder, chunked, content-length |

3. Config & CGI Developer

Implement configuration file parsing (port, route, error pages, etc.)

Configure static routes, uploads, error pages

Handle CGI execution (fork, execve)
| ConfigParser, routes, CGI, upload, directory listing, .php


🔧 Step 1: Core Developer builds the Minimal Server Framework
Goal: The browser should be able to connect to the server and receive a simple response like "Hello World".

Tasks:

Set up the socket and main poll() loop

Accept multiple client connections (even if you just reply with a fixed string)

Receive incoming request data (no need to parse it yet)

Send back a hardcoded HTTP response (e.g., a 200 OK with an HTML message)

✅ Once this basic server is working, the HTTP and Config developers can begin their parts in parallel.

🔀 Step 2: Parallel Development with Clear Interfaces
Module	Synchronization Point
✅ Core	After receiving request data, pass it to the RequestParser (HTTP module) and send back the resulting Response.
✅ HTTP	Parse the raw request (method, URI, headers, body), then use the Config module to match the correct route and build a valid response.
✅ Config	Provide lookup interfaces—e.g., given a URI, return route settings (upload path, supported methods, whether CGI is enabled, etc.).
