/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 15:30:41 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/01 12:12:11 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

int main(void)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // ecoute de n'importe quelle interface
    addr.sin_port = htons(8080); // est sur le port 8080
    
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, 10) < 0) { perror("listen"); return 1; } // listen a un backlog (??) de 10 
    
    std::cout <<  "Listening on http://localhost:8080" << std::endl;
    while (true) // peut etre a changer la cond de la boucle si erreur
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) { perror("accept"); continue; }
        
        char buffer[4096]; 
        ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0); // recois la requete
        if (n > 0) { buffer[n] = '\0'; std::cout << "--- Requete recue ---\n" << buffer << std::endl; }
        
        const char *body = "<h1> Hello (world) from c++98 </h1>";
        char response[512]; // en dessous, tout est ecrit basic, juste le content lenght et le body peuvent changer
        std::sprintf(response, "HTTPS/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Lenght: %lu\r\nConnection: close\r\n\r\n%s", (unsigned long)std::strlen(body), body);
        send(client_fd, response, std::strlen(response), 0);
        close(client_fd);
    }
    close(server_fd);
    return 0;
}