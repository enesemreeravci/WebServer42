#!/usr/bin/env python3
import os

print("Content-Type: text/plain\r")
print("\r")
print("Hello from CGI!")
print("Method:", os.environ.get("REQUEST_METHOD", "NONE"))
print("Query:", os.environ.get("QUERY_STRING", "NONE"))