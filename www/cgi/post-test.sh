#!/bin/bash
echo "Content-Type: text/html"
echo ""
echo "<!DOCTYPE html>"
echo "<html><head><title>POST Data Test</title></head><body>"
echo "<h1>POST Data Test</h1>"
echo "<p>Method: $REQUEST_METHOD</p>"
echo "<p>Content Length: $CONTENT_LENGTH</p>"

if [ "$REQUEST_METHOD" = "POST" ] && [ -n "$CONTENT_LENGTH" ]; then
    echo "<h2>POST Data Received:</h2>"
    echo "<pre>"
    # Read the POST data from stdin
    head -c $CONTENT_LENGTH
    echo "</pre>"
else
    echo "<p>No POST data received</p>"
fi

echo "</body></html>"