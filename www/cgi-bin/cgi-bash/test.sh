#!/bin/bash
echo "Content-Type: text/html"
echo ""
echo "<!DOCTYPE html>"
echo "<html><head><title>Shell CGI Test</title></head><body>"
echo "<h1>Shell CGI Working!</h1>"
echo "<p>Server: $SERVER_NAME</p>"
echo "<p>Method: $REQUEST_METHOD</p>"
echo "<p>URI: $REQUEST_URI</p>"
echo "<p>Time: $(date)</p>"
echo "<h2>Environment Variables:</h2>"
echo "<ul>"
env | sort | while IFS='=' read -r name value; do
    echo "<li><strong>$name</strong>: $value</li>"
done
echo "</ul>"
echo "</body></html>"