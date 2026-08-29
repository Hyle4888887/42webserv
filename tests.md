# Tests manuels — webserv

Tests à lancer depuis WSL (bash), dans le dossier du projet.
Nécessite `curl` et `nc` (netcat).

## 0. Build

```bash
make re
```

## 1. Lancer le serveur

```bash
mkdir -p uploads
./webserv config/conf_default &
```

Écoute sur `127.0.0.1:8080`.

## 2. GET statique

```bash
curl -s -i http://127.0.0.1:8080/
```

Attendu : `200 OK`, page d'accueil `www/index.html`.

## 3. 404 sur route inexistante

```bash
curl -s -i http://127.0.0.1:8080/nonexistent
```

Attendu : `404 Not Found`, page custom `errors/404.html`.

## 4. Upload de fichier (POST multipart)

```bash
echo 'hello webserv test' > /tmp/testupload.txt
curl -s -i -X POST -F 'file=@/tmp/testupload.txt' http://127.0.0.1:8080/uploads/
```

Attendu : `201 Created`.

## 5. Autoindex du dossier uploads

```bash
curl -s -i http://127.0.0.1:8080/uploads/
```

Attendu : `200 OK`, liste HTML avec `testupload.txt`.

## 6. DELETE du fichier uploadé

```bash
curl -s -i -X DELETE http://127.0.0.1:8080/uploads/testupload.txt
```

Attendu : `204 No Content`, puis re-vérifier `/uploads/` (le fichier ne doit plus apparaître).

⚠️ Ne teste pas DELETE sur des fichiers de `www/` (ex: `/index.html`) : la location `/`
autorise DELETE dans la config, donc ça supprimerait réellement le fichier tracké par git.
Reste sur `/uploads/...` pour ce test.

## 7. CGI en GET

```bash
curl -s -i http://127.0.0.1:8080/cgi-bin/hello.py
```

Attendu : `200 OK`, `<h1>Hello from CGI</h1>`.

## 8. CGI en POST

```bash
curl -s -i -X POST -d 'name=webserv&test=1' http://127.0.0.1:8080/cgi-bin/echo.py
```

Attendu : `200 OK`, script affiche method/query/length/body.

## 9. Redirect

```bash
curl -s -i http://127.0.0.1:8080/redirect
```

Attendu : `301 Moved Permanently`, `Location: https://example.com`.

## 10. Méthode non autorisée sur /cgi-bin (bug connu)

```bash
curl -s -i -X DELETE http://127.0.0.1:8080/cgi-bin/hello.py
curl -s -i -X PUT http://127.0.0.1:8080/cgi-bin/hello.py
```

Attendu : `405 Method Not Allowed` (la config limite `/cgi-bin` à `GET POST`).

**Bug détecté (2026-08-29)** : le serveur renvoie `200 OK` et exécute quand même le
script CGI, au lieu de vérifier `allowed_methods` avant de dispatcher vers le CGI.
À corriger, probablement dans `server/handleCGI.cpp` ou `server/handleOperation.cpp`.

## 11. Méthode non autorisée — test de contrôle (fonctionne)

```bash
curl -s -i -X PUT http://127.0.0.1:8080/
```

Attendu et obtenu : `405 Method Not Allowed`.

## 12. Body trop gros (client_max_body_size 1000000)

```bash
head -c 2000000 /dev/zero | tr '\0' 'a' > /tmp/big.txt
curl -s -i -X POST --data-binary @/tmp/big.txt http://127.0.0.1:8080/uploads/
```

Attendu : `413 Payload Too Large`.

## 13. Requête malformée (via nc, pas curl)

```bash
printf 'GARBAGE REQUEST HERE\r\n\r\n' | nc -w2 127.0.0.1 8080
```

Obtenu : `404` (traite "GARBAGE" comme méthode, "REQUEST" comme path inconnu).
Un `400 Bad Request` serait plus correct mais ce n'est pas bloquant.

## 14. Version HTTP invalide

```bash
printf 'GET / HTTP/9.9\r\nHost: localhost\r\n\r\n' | nc -w2 127.0.0.1 8080
```

Obtenu : `200 OK` (accepté sans vérifier la version — mineur).

## 15. Le serveur ne doit pas crasher après les requêtes malformées

```bash
curl -s -o /dev/null -w '%{http_code}\n' http://127.0.0.1:8080/
```

Attendu : `200` (serveur toujours vivant).

## 16. Arrêter le serveur

```bash
pkill -f './webserv config/conf_default'
```
