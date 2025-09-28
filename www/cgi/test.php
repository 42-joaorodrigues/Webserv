#!/usr/bin/php-cgi
<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<!DOCTYPE html><html><head><title>PHP CGI Test</title></head><body>";
echo "<h1>PHP CGI Working!</h1>";
echo "<p>Server: " . $_SERVER['SERVER_NAME'] . "</p>";
echo "<p>Method: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>URI: " . $_SERVER['REQUEST_URI'] . "</p>";
echo "<p>Time: " . date('Y-m-d H:i:s') . "</p>";
echo "</body></html>";
?>