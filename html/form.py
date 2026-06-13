#!/usr/bin/env python3
import os
import sys

method = os.environ.get("REQUEST_METHOD", "GET")
body = ""

if method == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
    if length > 0:
        body = sys.stdin.read(length)

print("Content-Type: text/html\r")
print("\r")
print("""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CGI Python — Form</title>
  <style>
    body { font-family: sans-serif; max-width: 700px; margin: 60px auto; color: #222; background: #f9f9f9; }
    form { background: #fff; border: 1px solid #ddd; border-radius: 6px; padding: 24px; }
    input[type="text"], textarea { width: 100%; padding: 8px; box-sizing: border-box; border: 1px solid #ccc; border-radius: 4px; margin: 8px 0 16px; font-size: 14px; }
    button { background: #0066cc; color: white; border: none; padding: 10px 24px; border-radius: 4px; cursor: pointer; }
    .result { background: #d4edda; border: 1px solid #c3e6cb; border-radius: 4px; padding: 14px; margin-top: 20px; color: #155724; }
    a { color: #0066cc; }
  </style>
</head>
<body>
  <h1>⚙️ CGI Python — Formulaire POST</h1>""")

if method == "POST" and body:
    print('<div class="result">')
    print('<strong>✅ Données reçues via POST :</strong><br>')
    print('<code>' + body + '</code>')
    print('</div>')

print("""
  <h2>Envoyer un message</h2>
  <form action="/cgi-bin/form.py" method="POST" enctype="application/x-www-form-urlencoded">
    <label><strong>Nom :</strong></label>
    <input type="text" name="name" placeholder="Ton nom">
    <label><strong>Message :</strong></label>
    <input type="text" name="message" placeholder="Hello webserv!">
    <button type="submit">Envoyer</button>
  </form>

  <p style="margin-top:30px"><a href="/index.html">← Retour à l'accueil</a></p>
</body>
</html>""")
