# WebServ Evaluation Checklist - Complete Verification

## ✅ Configuration Checks

### 1. Multiple Servers with Different Ports
**Status:** ✅ PASS
- Server 1: `127.0.0.1:8080` (server_name: webserv)
- Server 2: `127.0.0.1:8081` (server_name: youpibanane)

**Test commands:**
```bash
curl http://127.0.0.1:8080/
curl http://127.0.0.1:8081/
```

---

### 2. Multiple Servers with Different Hostnames
**Status:** ✅ CONFIGURED
- Server 1: `server_name webserv;`
- Server 2: `server_name youpibanane;`

**Test commands:**
```bash
# Test virtual hosting
curl --resolve webserv:8080:127.0.0.1 http://webserv:8080/
curl --resolve youpibanane:8081:127.0.0.1 http://youpibanane:8081/
```

---

### 3. Default Error Pages
**Status:** ✅ CONFIGURED
- 400 → `/errors/400/400.html`
- 403 → `/errors/403/403.html`
- 404 → `/errors/404/404.html`
- 413 → `/errors/413/413.html`

**Test commands:**
```bash
# Test 404
curl -v http://127.0.0.1:8080/nonexistent

# Test 403
curl -v http://127.0.0.1:8080/files/sub-directory

# Test custom error page content
curl http://127.0.0.1:8080/errors/404/404.html
```

---

### 4. Limit Client Body Size
**Status:** ✅ CONFIGURED
- Server default: `100M`
- `/post` location: `10` bytes (very restrictive for testing)
- `/post_body` (server 2): `100` bytes

**Test commands:**
```bash
# Should succeed (under 10 bytes)
curl -X POST -H "Content-Type: plain/text" --data "SHORT" http://127.0.0.1:8080/post/test

# Should fail with 413 (over 10 bytes)
curl -X POST -H "Content-Type: plain/text" --data "THIS IS A LONGER BODY THAT EXCEEDS THE LIMIT" http://127.0.0.1:8080/post/test

# Should succeed on server 2 (under 100 bytes)
curl -X POST -H "Content-Type: plain/text" --data "Medium length body here" http://127.0.0.1:8081/post_body
```

---

### 5. Routes to Different Directories
**Status:** ✅ CONFIGURED
- `/` → `./www`
- `/upload` → `./www/upload`
- `/post` → `./www/post`
- Server 2: `/` → `./YoupiBanane`
- Server 2: `/directory` → `./YoupiBanane` (alias)

**Test commands:**
```bash
curl http://127.0.0.1:8080/
curl http://127.0.0.1:8080/upload/
curl http://127.0.0.1:8081/
```

---

### 6. Default File for Directories
**Status:** ✅ CONFIGURED
- Server 1: `index index.html;`
- Server 2: `index youpi.bad_extension;`

**Test commands:**
```bash
# Should serve index.html
curl http://127.0.0.1:8080/

# Should serve youpi.bad_extension
curl http://127.0.0.1:8081/
```

---

### 7. Method Restrictions per Route
**Status:** ✅ CONFIGURED
- `/` → `GET` only
- `/post` → `POST` only
- `/upload` → `GET POST DELETE`
- `/cgi-bin/bash` → `POST` only
- `/cgi-bin/python` → `GET POST`

**Test commands:**
```bash
# Should succeed (GET allowed on /)
curl -X GET http://127.0.0.1:8080/

# Should fail with 405 (POST not allowed on /)
curl -X POST http://127.0.0.1:8080/

# Should succeed (DELETE allowed on /upload)
curl -X DELETE http://127.0.0.1:8080/upload/test.txt

# Should fail (DELETE not allowed on /)
curl -X DELETE http://127.0.0.1:8080/test.txt
```

---

## ✅ Basic HTTP Checks

### 1. GET Requests
**Status:** ✅ TO TEST

**Test commands:**
```bash
curl -v http://127.0.0.1:8080/
curl -v http://127.0.0.1:8080/index.html
curl -v http://127.0.0.1:8080/about/
```

---

### 2. POST Requests
**Status:** ✅ TO TEST

**Test commands:**
```bash
# Simple POST
curl -X POST -d "test=data" http://127.0.0.1:8080/post/hello

# File upload
curl -X POST -F "file=@/path/to/file.txt" http://127.0.0.1:8080/upload
```

---

### 3. DELETE Requests
**Status:** ✅ TO TEST

**Test commands:**
```bash
# First create a file
echo "test content" > /tmp/upload_test.txt
curl -X POST -F "file=@/tmp/upload_test.txt" http://127.0.0.1:8080/upload

# Then delete it
curl -X DELETE http://127.0.0.1:8080/upload/upload_test.txt
```

---

### 4. Unknown Methods Don't Crash
**Status:** ✅ TO TEST

**Test commands:**
```bash
# Using telnet
telnet 127.0.0.1 8080
UNKNOWN / HTTP/1.1
Host: localhost

# Or using curl
curl -X TRACE http://127.0.0.1:8080/
curl -X OPTIONS http://127.0.0.1:8080/
curl -X PATCH http://127.0.0.1:8080/
```

**Expected:** Server should return 405 Method Not Allowed or similar, NOT crash

---

### 5. Correct Status Codes
**Status:** ✅ TO TEST

**Test matrix:**
| Test Case | Expected Status | Test Command |
|-----------|----------------|--------------|
| Valid GET | 200 OK | `curl -v http://127.0.0.1:8080/` |
| Not Found | 404 Not Found | `curl -v http://127.0.0.1:8080/nonexistent` |
| Method Not Allowed | 405 | `curl -X POST -v http://127.0.0.1:8080/` |
| Payload Too Large | 413 | `curl -X POST --data "LONG_DATA" http://127.0.0.1:8080/post/test` |
| Redirect | 301 | `curl -v http://127.0.0.1:8080/redirect` |
| Forbidden | 403 | `curl -v http://127.0.0.1:8080/files/sub-directory` |

---

### 6. File Upload and Download
**Status:** ✅ TO TEST

**Test commands:**
```bash
# Upload a file
echo "Hello WebServ" > /tmp/test_upload.txt
curl -X POST -F "file=@/tmp/test_upload.txt" http://127.0.0.1:8080/upload

# Download it back
curl http://127.0.0.1:8080/upload/test_upload.txt

# Verify content matches
curl -s http://127.0.0.1:8080/upload/test_upload.txt | diff - /tmp/test_upload.txt
```

---

## ✅ CGI Checks

### 1. CGI Works Properly
**Status:** ✅ CONFIGURED (2 CGI systems)
- Bash CGI: `/cgi-bin/bash/` → `/bin/bash` (`.sh` extension)
- Python CGI: `/cgi-bin/python/` → `/usr/bin/python3` (`.py` extension)

**Available CGI scripts:**
- `www/cgi-bin/cgi-bash/test.sh`
- `www/cgi-bin/cgi-python/test.py`
- `www/cgi-bin/cgi-python/toupper.py`
- `www/cgi-bin/cgi-python/session.py`
- `www/cgi-bin/bash/create_file.sh`

---

### 2. CGI with GET Method
**Status:** ✅ TO TEST

**Test commands:**
```bash
# Python CGI with GET
curl http://127.0.0.1:8080/cgi-bin/python/test.py

# Session management (GET)
curl http://127.0.0.1:8080/cgi-bin/python/session.py
```

---

### 3. CGI with POST Method
**Status:** ✅ TO TEST

**Test commands:**
```bash
# Python toupper CGI
curl -X POST -d "hello world" http://127.0.0.1:8080/cgi-bin/python/toupper.py

# Bash create_file CGI
curl -X POST -d "size=10K" http://127.0.0.1:8080/cgi-bin/bash/create_file.sh
```

---

### 4. CGI Runs in Correct Directory
**Status:** ✅ TO VERIFY
- Must test relative path access from CGI scripts
- Check if CGI can access files relative to its location

**Test:** Create a CGI script that reads a file in the same directory using relative path

---

### 5. CGI Error Handling
**Status:** ✅ TO TEST

**Test scenarios:**
1. **Infinite loop script:**
   ```bash
   # Create a script with infinite loop (should timeout)
   echo '#!/bin/bash\nwhile true; do echo "infinite"; done' > /tmp/infinite.sh
   chmod +x /tmp/infinite.sh
   # Copy to CGI directory and test
   ```

2. **Script with errors:**
   ```bash
   # Create a script with syntax errors
   echo '#!/usr/bin/python3\nprint(undefined_variable)' > /tmp/error.py
   # Test and verify server doesn't crash
   ```

3. **Non-executable script:**
   ```bash
   # Upload a script without execute permissions
   ```

**Expected:** Server should return 500 Internal Server Error, NOT crash

---

## ✅ Browser Compatibility

### 1. Serve Static Website
**Status:** ✅ TO TEST
- Open: `http://127.0.0.1:8080/` in browser
- Check: Network tab shows correct headers
- Verify: All static resources load (HTML, CSS, JS, images)

**Check for:**
- Correct Content-Type headers
- Correct Content-Length headers
- Keep-Alive connection handling

---

### 2. Wrong URL Handling
**Status:** ✅ TO TEST
- Open: `http://127.0.0.1:8080/this-does-not-exist`
- Expected: Custom 404 error page displays

---

### 3. Directory Listing
**Status:** ✅ CONFIGURED (autoindex on `/upload`)
- Open: `http://127.0.0.1:8080/upload/`
- Expected: Directory listing shows uploaded files

---

### 4. Redirect URL
**Status:** ✅ CONFIGURED
- Open: `http://127.0.0.1:8080/redirect`
- Expected: Browser redirects to `http://127.0.0.1:8080`
- Check: Network tab shows 301 Moved Permanently

---

## ✅ Port Configuration

### 1. Multiple Ports Work
**Status:** ✅ CONFIGURED
- Port 8080: Main server (webserv)
- Port 8081: Test server (youpibanane)

**Test:**
```bash
# Both should work simultaneously
curl http://127.0.0.1:8080/ &
curl http://127.0.0.1:8081/ &
```

---

### 2. Same Port Multiple Times Fails
**Status:** ✅ TO TEST

**Test:** Create a config with duplicate port:
```
server {
    listen 127.0.0.1:8080;
}
server {
    listen 127.0.0.1:8080;
}
```

**Expected:** Server should fail to start with appropriate error message

---

### 3. Multiple Servers with Common Ports
**Status:** ✅ TO TEST

**Test:** Start two separate server instances with the same config

**Expected:** Second instance should fail to bind to port (address already in use)

---

## ✅ Stress Testing (Siege)

### Installation
```bash
# Install siege
brew install siege

# Or on Linux
apt-get install siege
```

### Tests to Run

#### 1. Availability Test (>99.5% required)
```bash
# Test on empty page for 60 seconds
siege -b -t60s http://127.0.0.1:8080/

# Expected output should show:
# Availability: > 99.50%
```

#### 2. Memory Leak Test
```bash
# Run siege in background
siege -b -c100 -t300s http://127.0.0.1:8080/ &

# Monitor memory in another terminal
watch -n 1 'ps aux | grep webserv | grep -v grep'

# Memory (RSS) should remain stable, not grow indefinitely
```

#### 3. No Hanging Connections
```bash
# Run siege
siege -b -c50 -t120s http://127.0.0.1:8080/

# Check connections
netstat -an | grep 8080 | grep ESTABLISHED
# Should be 0 or very few after siege completes

# Check for TIME_WAIT (should drain normally)
netstat -an | grep 8080 | grep TIME_WAIT
```

#### 4. Indefinite Siege
```bash
# This should run without crashing
siege -b http://127.0.0.1:8080/

# Let it run for several minutes, press Ctrl+C to stop
# Server should still be responsive after
```

---

## ✅ Bonus Features

### 1. Cookies and Session Management
**Status:** ✅ IMPLEMENTED
- Location: `www/cgi-bin/python/session.py`
- Creates session ID with UUID
- Stores session data in `sessions/sessions.json`
- Sets cookies with expiration

**Test:**
```bash
# Get a session cookie
curl -v http://127.0.0.1:8080/cgi-bin/python/session.py 2>&1 | grep Set-Cookie

# Use the cookie in subsequent requests
curl -b "session_id=YOUR_SESSION_ID" http://127.0.0.1:8080/cgi-bin/python/session.py
```

**Browser test:**
- Open: `http://127.0.0.1:8080/tests/`
- Click "Take The Cookie Anyways" button
- Verify session is created and persists across page reloads

---

### 2. Multiple CGI Systems
**Status:** ✅ IMPLEMENTED
- **Python CGI:** `/usr/bin/python3` (`.py` extension)
  - `test.py`
  - `toupper.py`
  - `session.py`
  
- **Bash CGI:** `/bin/bash` (`.sh` extension)
  - `create_file.sh`
  - `test.sh`

**Test both:**
```bash
# Python CGI
curl -X POST -d "test data" http://127.0.0.1:8080/cgi-bin/python/toupper.py

# Bash CGI
curl -X POST -d "size=1K" http://127.0.0.1:8080/cgi-bin/bash/create_file.sh
```

---

## 📋 Evaluation Questions to Prepare For

### HTTP Server Basics
**Q:** Explain how an HTTP server works.
**A:** Should cover:
- Client connects via TCP socket
- Server accepts connection
- Client sends HTTP request (method, URI, headers, body)
- Server parses request, processes it, generates response
- Server sends HTTP response (status line, headers, body)
- Connection closed or kept alive for next request

### I/O Multiplexing
**Q:** What function did you use for I/O Multiplexing?
**A:** `epoll` (Linux) - specifically `epoll_create`, `epoll_ctl`, and `epoll_wait`

**Q:** Explain how epoll works.
**A:** Should cover:
- Creates an epoll instance with `epoll_create`
- Registers file descriptors to monitor with `epoll_ctl` (EPOLL_CTL_ADD)
- Waits for events with `epoll_wait` (blocks until events occur)
- Returns array of events that are ready
- More efficient than select/poll for many file descriptors

### Single epoll_wait
**Q:** Do you use only one epoll_wait?
**A:** Yes, in the main loop at `src/network/epoll.cpp:157` in function `waitEpoll()`

**Q:** How do you manage accept and read/write?
**A:** 
- `epoll_wait` monitors all file descriptors (listening sockets and client sockets)
- When listening socket has EPOLLIN → call `accept()` to get new client
- When client socket has EPOLLIN → call `recv()` to read data
- When client socket has EPOLLOUT → call `send()` to write data
- All in one epoll loop

### Code Path
**Q:** Show the code from epoll to read/write of a client.
**A:** Path is:
1. `epoll_wait()` returns events → `src/network/epoll.cpp:157`
2. Loop processes events → `src/network/epoll.cpp:167`
3. If EPOLLIN on client socket → `HttpHandler::handleHttpRequest()` → `src/network/epoll.cpp:200`
4. Inside handler → `readFullHttpRequest()` calls `recv()` → `src/http/HttpHandler.cpp:~412`
5. Response sent with `send()` → `src/http/HttpHandler.cpp:~748`

---

## 🎯 Final Checklist Before Evaluation

- [ ] Compile with `make` - no errors, no re-link
- [ ] Start server with default config
- [ ] Start server with custom config
- [ ] Test all curl commands above
- [ ] Test in browser (Chrome/Firefox)
- [ ] Run siege stress tests
- [ ] Verify no memory leaks with `top` or `htop`
- [ ] Check no hanging connections with `netstat`
- [ ] Test CGI scripts work
- [ ] Test error handling (invalid requests, missing files, etc.)
- [ ] Test session/cookies in browser
- [ ] Prepare answers to evaluation questions

---

## ⚠️ Common Pitfalls to Avoid

1. **Don't crash on:**
   - Invalid HTTP requests
   - Large file uploads
   - Rapid connections/disconnections
   - Malformed headers
   - Unknown HTTP methods

2. **Always return correct status codes:**
   - 200 OK
   - 301 Moved Permanently
   - 400 Bad Request
   - 403 Forbidden
   - 404 Not Found
   - 405 Method Not Allowed
   - 413 Payload Too Large
   - 500 Internal Server Error

3. **File operations:**
   - Always check if file exists before serving
   - Handle permission errors
   - Close file descriptors properly

4. **Connection management:**
   - Close connections on errors
   - Remove from epoll when closing
   - Handle keep-alive properly

5. **CGI:**
   - Set environment variables correctly
   - Handle CGI timeouts
   - Clean up CGI processes
   - Parse CGI output headers correctly
