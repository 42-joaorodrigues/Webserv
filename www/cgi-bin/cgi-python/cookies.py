#!/usr/bin/env python3

import cgi
import os
from http import cookies


cookie = cookies.SimpleCookie()
if "HTTP_COOKIE" in os.environ:
    cookie.load(os.environ["HTTP_COOKIE"])

if "session_id" in cookie:
    session_id = cookie["session_id"].value
else:
    import uuid
    session_id = uuid.uuid4()
    cookie["session_id"] = session_id
    cookie["session_id"]["max-age"] = 3600 #1 hour length for cookie
    cookie["session_id"]["path"] = "/"

print("Content-Type: text/html")
print(cookie.output())  # Set-Cookie header
print()
print(f"<html><body><h1>Session ID: {session_id}</h1></body></html>")