/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   startCGI.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 16:23:14 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/04 15:00:20 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

static std::string toString(unsigned long value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

static void executeChild(const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &body)
{
    std::string dir = ".", file = scriptPath; std::string::size_type slash = scriptPath.find_last_of('/');
    if (slash != std::string::npos) { dir = scriptPath.substr(0, slash); file = scriptPath.substr(slash + 1); if (dir.empty()) {dir = "/";}}
    // env version mini
    std::vector<std::string> env;
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("REQUEST_METHOD=" + method);
    env.push_back("QUERY_STRING=" + query);
    env.push_back("SCRIPT_FILENAME=" + file);
    env.push_back("REDIRECT_STATUS=200");
    if (method == "POST")
    {
        env.push_back("CONTENT_LENGTH=" + toString(body.size()));
        env.push_back("CONTENT_TYPE=application/x-ww-form-urlencoded");
    } // end
    
    std::vector<char*> envp;
    for (size_t i = 0; i < env.size(); i++) { envp.push_back(const_cast<char*>(env[i].c_str())); }
    envp.push_back(NULL);
    
    char* argv[] = {const_cast<char*>(interpreter.c_str()), const_cast<char*>(file.c_str()), NULL};
    if (chdir(dir.c_str()) != 0) {std::exit(1);}
    execve(interpreter.c_str(), argv, &envp[0]);
    std::exit(1);
}

static void closeIt(int fd[2]) { close(fd[0]); close(fd[1]); }

bool CGI::start(const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &body)
{
    _pid = -1; _fdIn = -1; _fdOut = -1;
    int in[2], out[2];
    if (pipe(in) < 0) { return false; }
    if (pipe(out) < 0) { closeIt(in); return false; }
    _pid = fork();
    if (_pid < 0) { closeIt(in); closeIt(out); return false; }
    if (_pid == 0)
    {
        dup2(in[0], STDIN_FILENO); dup2(out[1], STDOUT_FILENO);
        closeIt(in); closeIt(out);
        executeChild(interpreter, scriptPath, method, query, body);
        std::exit(1);
    }
    close(in[0]); close(out[1]);
    fcntl(in[1], F_SETFL, O_NONBLOCK);
    fcntl(out[0], F_SETFL, O_NONBLOCK);
    _fdOut = out[0];
    if (body.empty()) { close(in[1]); _fdIn = -1; }
    else { _fdIn = in[1]; }
    return true;
}