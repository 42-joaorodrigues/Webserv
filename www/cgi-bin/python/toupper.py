#!/usr/bin/python3
import os
import sys

# Read POST body if present
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
body = ""

if content_length > 0:
    body = sys.stdin.read(content_length)

# Return body in uppercase
print("Content-Type: text/plain")
print()  # Empty line required after headers
print(body.upper())