#include "CGI.hpp"

extern char **environ;

// Return the "KEY" part of a "KEY=VALUE" environment entry.
static std::string envKey(const std::string &entry)
{
    std::string::size_type eq = entry.find('=');
    return (eq == std::string::npos) ? entry : entry.substr(0, eq);
}

static std::string interpreterPathFromScriptDirectory(const std::string &interpreter, const std::string &directory)
{
    if (interpreter.empty() || interpreter[0] == '/')
        return interpreter;

    std::string prefix;
    std::size_t begin = 0;
    while (begin < directory.size())
    {
        std::size_t end = directory.find('/', begin);
        if (end == std::string::npos)
            end = directory.size();
        std::string component = directory.substr(begin, end - begin);
        if (!component.empty() && component != ".")
            prefix += "../";
        begin = end + 1;
    }
    return prefix + interpreter;
}

// Prepare and execute the CGI child process.
static void executeChild(const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &requestUri, const std::string &body, const std::map<std::string, std::string> &headers, const std::string &serverName, const std::string &serverPort)
{
    std::string dir = ".", file = scriptPath; std::string::size_type slash = scriptPath.find_last_of('/');
    if (slash != std::string::npos) { dir = scriptPath.substr(0, slash); file = scriptPath.substr(slash + 1); if (dir.empty()) {dir = "/";}}
    std::string execPath = interpreterPathFromScriptDirectory(interpreter, dir);
    std::vector<std::string> env;
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("REQUEST_METHOD=" + method);
    env.push_back("QUERY_STRING=" + query);
    env.push_back("REQUEST_URI=" + requestUri);
    env.push_back("PATH_INFO=" + requestUri);
    env.push_back("SERVER_NAME=" + serverName);
    env.push_back("SERVER_PORT=" + serverPort);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("SCRIPT_NAME=" + file);
    env.push_back("REDIRECT_STATUS=200");
    std::map<std::string, std::string>::const_iterator contentLength = headers.find("content-length");
    if (contentLength != headers.end())
        env.push_back("CONTENT_LENGTH=" + contentLength->second);
    else if (method == "POST")
        env.push_back("CONTENT_LENGTH=" + toString(body.size()));
    std::map<std::string, std::string>::const_iterator contentType = headers.find("content-type");
    if (contentType != headers.end())
        env.push_back("CONTENT_TYPE=" + contentType->second);
    else if (method == "POST")
        env.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");

    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        if (it->first == "content-length" || it->first == "content-type")
            continue;
        std::string name = it->first;
        for (std::size_t i = 0; i < name.size(); ++i)
        {
            if (name[i] == '-')
                name[i] = '_';
            else if (name[i] >= 'a' && name[i] <= 'z')
                name[i] = static_cast<char>(name[i] - ('a' - 'A'));
        }
        env.push_back("HTTP_" + name + "=" + it->second);
    }

    // Inherit the server's own environment (PATH, LANG, TZ, ...) for anything
    // the meta-variables above don't already define, so scripts that shell
    // out or rely on the interpreter's normal runtime setup still work.
    for (char **e = environ; e != NULL && *e != NULL; ++e)
    {
        std::string entry(*e);
        std::string key = envKey(entry);
        bool overridden = false;
        for (std::size_t i = 0; i < env.size(); ++i)
        {
            if (envKey(env[i]) == key) { overridden = true; break; }
        }
        if (!overridden)
            env.push_back(entry);
    }

    std::vector<char*> envp;
    for (size_t i = 0; i < env.size(); i++) { envp.push_back(const_cast<char*>(env[i].c_str())); }
    envp.push_back(NULL);
    
    char* argv[] = {const_cast<char*>(execPath.c_str()), const_cast<char*>(file.c_str()), NULL};
    if (chdir(dir.c_str()) != 0) {std::exit(1);}
    execve(execPath.c_str(), argv, &envp[0]);
    std::exit(1);
}

// Close both ends of a pipe pair.
static void closeIt(int fd[2]) { close(fd[0]); close(fd[1]); }

// Start CGI execution using pipes and fork.
bool CGI::start(const std::string &interpreter, const std::string &scriptPath, const std::string &method, const std::string &query, const std::string &requestUri, const std::string &body, const std::map<std::string, std::string> &headers, const std::string &serverName, const std::string &serverPort)
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
        executeChild(interpreter, scriptPath, method, query, requestUri, body, headers, serverName, serverPort);
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