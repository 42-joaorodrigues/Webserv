#!/usr/bin/python3
import os
import sys
from datetime import datetime

# CGI Header
print("Content-Type: text/html")
print()  # Empty line required after headers

# HTML Content
print("<!DOCTYPE html>")
print("<html><head><title>Python CGI Test</title></head><body>")
print("<h1>Python CGI Working!</h1>")
print(f"<p>Server: {os.environ.get('SERVER_NAME', 'Unknown')}</p>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD', 'Unknown')}</p>")
print(f"<p>URI: {os.environ.get('REQUEST_URI', 'Unknown')}</p>")
print(f"<p>Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>")
print(f"<p>Python Version: {sys.version}</p>")

print("<h2>Environment Variables:</h2>")
print("<ul>")
for key in sorted(os.environ.keys()):
    value = os.environ[key]
    print(f"<li><strong>{key}</strong>: {value}</li>")
print("</ul>")

# If POST data exists, show it
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print("<h2>POST Data:</h2>")
        print(f"<pre>{post_data}</pre>")

print("</body></html>")