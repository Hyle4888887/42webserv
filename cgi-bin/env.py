#!/usr/bin/env python3
import os, sys
body = sys.stdin.read()
print("Content-Type: text/plain")
print()
for k in sorted(os.environ):
    print(k, "=", os.environ[k])
print("BODY =", body)