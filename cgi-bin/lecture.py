#!/usr/bin/env python3
print("Content-Type: text/plain")
print()
with open("data.txt") as f:
    print(f.read())