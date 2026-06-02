/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 15:30:41 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/02 15:09:47 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

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
        if (n <= 0) { close(client_fd); continue; }
        buffer[n] = '\0'; std::cout << "--- Requete recue ---\n" << buffer << std::endl;
        
        //Parsing mini
        std::string rawRequest(buffer, n); std::string method, target;
        std::istringstream lineStream(rawRequest); lineStream >> method >> target;
        
        std::string path = target, query; std::string::size_type q = target.find('?');
        if (q != std::string::npos) { path = target.substr(0, q); query = target.substr(q + 1); }
        std::string body; std::string::size_type bsep = rawRequest.find("\r\n\r\n");
        if (bsep != std::string::npos) { body = rawRequest.substr(bsep + 4); }
        //end parse
        
        //partie CGI -- ne pas toucher
        bool isCGI = false; std::string interpreter; std::string::size_type dot = path.rfind('.');
        if (dot != std::string::npos)
        {
            std::string ext = path.substr(dot);
            if (ext == ".py") { isCGI = true; interpreter  = "/usr/bin/python3"; }
            /*else if (ext == ".php") { isCGI = true; interpreter  = "/usr/bin/php-cgi"; }*/
        }
        if (isCGI) {
            std::string scriptPath = "." + path; std::string cgiOut = executeCGI(interpreter, scriptPath, method, query, body);
            std::string res = buildCGIResponse(cgiOut);
            send(client_fd, res.c_str(), res.size(), 0);
        } else {
            //peut etre changer c'est juste un truc qui teste
            std::string body = "<h1> Hello (world) from c++98 </h1>";
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n" << "Content-Type: text/html\r\n" << "Content-Lenght: " << body.size() << "\r\n" << "Connection: close\r\n" << "\r\n" << body;
            std::string raw = response.str();
            send(client_fd, raw.c_str(), raw.size(), 0);
        }
        close(client_fd);
    }
    close(server_fd);
    return 0;
}