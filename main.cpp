/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 15:30:41 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/01 14:29:40 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cerrno>

int main(void)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { std::cerr << "socket: " << std::strerror(errno) << std::endl; return 1; }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr = sockaddr_in();
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // ecoute de n'importe quelle interface
    addr.sin_port = htons(8080); // est sur le port 8080
    
    if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) { std::cerr << "bind: " << std::strerror(errno) << std::endl; close(server_fd); return 1; }
    if (listen(server_fd, 10) < 0) { std::cerr << "listen: " << std::strerror(errno) << std::endl; close(server_fd); return 1; } // listen a un backlog (??) de 10 
    
    std::cout <<  "Listening on http://localhost:8080" << std::endl;
    while (true) // peut etre a changer la cond de la boucle si erreur
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) { std::cerr << "accept: " << std::strerror(errno) << std::endl; continue; }
        
        char buffer[4096]; 
        ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0); // recois la requete
        if (n > 0) { buffer[n] = '\0'; std::cout << "--- Requete recue ---\n" << buffer << std::endl; }
        
        std::string body = "<h1> Hello (world) from c++98 </h1>";
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n" << "Content-Type: text/html\r\n" << "Content-Lenght: " << body.size() << "\r\n" << "Connection: close\r\n" << "\r\n" << body;
        std::string raw = response.str();
        send(client_fd, raw.c_str(), raw.size(), 0);
        close(client_fd);
    }
    close(server_fd);
    return 0;
}