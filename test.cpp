/*
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

int main()
{
    // 1. Creer le socket serveur
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Configurer l'adresse
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(4242);

    // 3. Bind + listen
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1);

    std::cout << "En attente sur http://localhost:4242" << std::endl;

    // 4. Attendre UNE connexion (boucle simple, pas encore de poll)
    int client_fd = -1;
    while (client_fd < 0)
        client_fd = accept(server_fd, NULL, NULL);

    std::cout << "Client connecte !" << std::endl;

    // 5. Lire ce que le client envoie
    char buffer[1024];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
        buffer[n] = '\0';
        std::cout << "Recu :\n" << buffer << std::endl;
    }

    close(client_fd);
    close(server_fd);
    return 0;
}*/

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 13:33:16 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/01 13:51:05 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>


std::string toString(long n) { std::ostringstream oss; oss << n; return oss.str(); }

std::vector<std::string> buildEnv(const Request &req)
{
    std::vector<std::string> env;
    // Implementation for building environment variables
    return env;
}*/
