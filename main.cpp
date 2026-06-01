/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 16:33:56 by bozil             #+#    #+#             */
/*   Updated: 2026/06/01 16:33:59 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>

#define MAX_CLIENTS 64
#define PORT        8080

// Rend un fd non-bloquant
static void set_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

// Cree et configure un socket d'ecoute sur le port donne
static int create_listening_socket(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(server_fd);

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return -1;
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        close(server_fd);
        return -1;
    }
    return server_fd;
}

// Envoie une reponse HTTP minimale et ferme la connexion
static void send_response(int client_fd, const char *body)
{
    char response[1024];
    std::sprintf(response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        (unsigned long)std::strlen(body),
        body
    );
    send(client_fd, response, std::strlen(response), 0);
}

int main(void)
{
    // --- Etape 1 : creation du socket serveur ---
    int server_fd = create_listening_socket(PORT);
    if (server_fd < 0) return 1;
    std::cout << "Listening on http://localhost:" << PORT << std::endl;

    // --- Etape 2 : tableau de pollfd ---
    // Index 0 = socket serveur (POLLIN uniquement)
    // Index 1..N = clients connectes (POLLIN + POLLOUT selon l'etat)
    std::vector<struct pollfd> fds;

    struct pollfd server_pfd;
    server_pfd.fd      = server_fd;
    server_pfd.events  = POLLIN;
    server_pfd.revents = 0;
    fds.push_back(server_pfd);

    // --- Boucle principale ---
    while (true)
    {
        // poll() bloque jusqu'a ce qu'un fd soit pret
        int ready = poll(fds.data(), (nfds_t)fds.size(), -1);
        if (ready < 0) { perror("poll"); break; }

        // --- Etape 3 : nouvelle connexion sur le socket serveur ---
        if (fds[0].revents & POLLIN)
        {
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd >= 0)
            {
                set_nonblocking(client_fd);

                struct pollfd client_pfd;
                client_pfd.fd      = client_fd;
                client_pfd.events  = POLLIN; // on attend d'abord la requete
                client_pfd.revents = 0;
                fds.push_back(client_pfd);

                std::cout << "[+] Nouveau client fd=" << client_fd
                          << "  (total: " << fds.size() - 1 << ")" << std::endl;
            }
        }

        // --- Etape 4 : traitement des clients ---
        for (std::size_t i = 1; i < fds.size(); ++i)
        {
            int fd = fds[i].fd;

            // Donnees disponibles en lecture
            if (fds[i].revents & POLLIN)
            {
                char buffer[4096];
                ssize_t n = recv(fd, buffer, sizeof(buffer) - 1, 0);

                if (n <= 0)
                {
                    // n == 0 : client a ferme la connexion
                    // n <  0 : erreur (EAGAIN ne devrait pas arriver ici car poll a signale POLLIN)
                    std::cout << "[-] Client fd=" << fd << " deconnecte" << std::endl;
                    close(fd);
                    fds.erase(fds.begin() + i);
                    --i;
                    continue;
                }

                buffer[n] = '\0';
                std::cout << "--- Requete recue (fd=" << fd << ") ---\n"
                          << buffer << std::endl;

                // Une fois la requete recue, on passe en mode POLLOUT pour envoyer
                // la reponse (ici on fait tout en une fois pour simplifier,
                // en vrai il faudrait un buffer de sortie)
                const char *body = "<h1>Hello from webserv (poll version)</h1>";
                send_response(fd, body);

                // Fermeture propre apres envoi (Connection: close)
                close(fd);
                fds.erase(fds.begin() + i);
                --i;
            }
            // Cas d'erreur sur le fd client
            else if (fds[i].revents & (POLLHUP | POLLERR))
            {
                std::cout << "[-] Erreur/HUP fd=" << fd << std::endl;
                close(fd);
                fds.erase(fds.begin() + i);
                --i;
            }
        }
    }

    close(server_fd);
    return 0;
}