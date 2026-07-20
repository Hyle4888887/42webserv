#!/usr/bin/env python3
import os
import datetime

print("Content-Type: text/html\r")
print("\r")
print("""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CGI Python — Hello</title>
  <style>
    body { font-family: sans-serif; max-width: 700px; margin: 60px auto; color: #222; background: #f9f9f9; }
    table { border-collapse: collapse; width: 100%; margin-top: 16px; }
    th, td { border: 1px solid #ddd; padding: 8px 12px; text-align: left; }
    th { background: #f0f0f0; }
    a { color: #0066cc; }
  </style>
</head>
<body>
  <h1>⚙️ CGI Python — Hello</h1>
  <p>Ce script CGI tourne côté serveur. Si tu vois cette page, le CGI Python fonctionne.</p>
  <p><strong>Heure serveur :</strong> """ + str(datetime.datetime.now()) + """</p>

  <h2>Variables d'environnement CGI</h2>
  <table>
    <tr><th>Variable</th><th>Valeur</th></tr>""")

cgi_vars = [
    "REQUEST_METHOD", "SCRIPT_FILENAME", "PATH_INFO",
    "QUERY_STRING", "CONTENT_TYPE", "CONTENT_LENGTH",
    "SERVER_NAME", "SERVER_PORT", "HTTP_HOST", "REDIRECT_STATUS"
]

for var in cgi_vars:
    val = os.environ.get(var, "<non définie>")
    print("    <tr><td><code>" + var + "</code></td><td>" + val + "</td></tr>")

print("""  </table>
  <p style="margin-top:30px"><a href="/index.html">← Retour à l'accueil</a></p>
</body>
</html>""")
