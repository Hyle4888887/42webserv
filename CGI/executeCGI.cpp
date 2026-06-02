/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executeCGI.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpoirier <mpoirier@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 16:23:14 by mpoirier          #+#    #+#             */
/*   Updated: 2026/06/02 13:31:44 by mpoirier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserv.hpp"

std::string executeCGI(const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &body)
{
    std::string scriptDir, scriptFile; std::string::size_type slash = scriptPath.find_last_of('/');
    if (slash == std::string::npos) { scriptDir = "."; scriptFile = scriptPath; }
    else { scriptDir = scriptPath.substr(0, slash); scriptFile = scriptPath.substr(slash + 1); if (scriptDir.empty()) {scriptDir = "/";}}
    // env version mini
    std::vector<std::string> env;
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("REQUEST_METHOD=" + method);
    env.push_back("QUERY_STRING=" + query);
    env.push_back("SCRIPT_FILENAME=" + scriptFile);
    env.push_back("REDIRECT_STATUS=200");
    if (method == "POST") {
        std::ostringstream len; len << body.size();
        env.push_back("CONTENT_LENGTH=" + len.str());
        env.push_back("CONTENT_TYPE=application/x-ww-form-urlencoded");
    } // end
    
    std::vector<char*> envp;
    for (size_t i = 0; i < env.size(); i++) { envp.push_back(const_cast<char*>(env[i].c_str())); }
    envp.push_back(NULL);
    char* argv[] = {const_cast<char*>(interpreter.c_str()), const_cast<char*>(scriptFile.c_str()), NULL};
    int in[2], out[2];
    if (pipe(in) < 0 || pipe(out) < 0) { return ""; }
    pid_t pid = fork();
    if (pid < 0) { return ""; }
    else if (pid == 0) {
        dup2(in[0], STDIN_FILENO); close(in[0]); close(in[1]);
        dup2(out[1], STDOUT_FILENO); close(out[0]); close(out[1]);
        if (chdir(scriptDir.c_str()) != 0) {std::exit(1);}
        execve(interpreter.c_str(), argv, &envp[0]);
        std::exit(1);
    }
    close(in[0]); close(out[1]);
    if (!body.empty()) { write(in[1], body.c_str(), body.size()); }
    close(in[1]);

    std::string output; char buf[4096]; ssize_t r;
    while ((r = read(out[0], buf, size_t(buf))) > 0) { output.append(buf, r); }
    close(out[0]);
    
    waitpid(pid, NULL, 0); return output;
}