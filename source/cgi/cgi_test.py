import sys

body = sys.stdin.read()

print("Content-Type: text/plain\n")
print("Hello from Python File!")
print("Received body:", body)