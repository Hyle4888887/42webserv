
#include "webserv.hpp"
#include "parser/configParser.hpp"

volatile sig_atomic_t g_stop = 0;

static void handleStopSignal(int)
{
	g_stop = 1;
}

// Program entry point.
int	main(int argc, char **argv, char **envp)
{
	if (argc > 3)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return (1);
	}
	
	try
	{
		std::string configPath = (argc == 2) ? argv[1] : "config/conf_default";
		ConfigParser parser(configPath);
		std::cout << "Configuration parsed successfully!" << std::endl;

		const Config& config = parser.getConfig();

		signal(SIGPIPE, SIG_IGN);
		signal(SIGINT, handleStopSignal);
		signal(SIGTERM, handleStopSignal);

		Server	server(config);
		if (config.servers.empty())
		{
			std::cerr << "No server blocks found." << std::endl;
			return (1);
		}

		std::vector<std::string> listeners;
		for (std::size_t i = 0; i < config.servers.size(); ++i)
		{
			int port = config.servers[i].port;
			std::string listenerKey = config.servers[i].host + ":" + toString(static_cast<unsigned long>(port));
			bool seen = false;
			for (std::size_t j = 0; j < listeners.size(); ++j)
			{
				if (listeners[j] == listenerKey)
				{
					seen = true;
					break;
				}
			}
			if (!seen)
			{
				if (!server.addListener(config.servers[i].host, port))
					return 1;
				listeners.push_back(listenerKey);
			}
			else
			{
				std::cerr << "Duplicate listen directive: " << listenerKey << std::endl;
				return 1;
			}
		}

		server.run();
		char arg0[] = "/bin/ls";
    	char arg1[] = "-la";
    	char *args[] = { arg0, arg1, NULL };
		execve(args[0], args, envp);
		if (execve(args[0], args, envp) == -1)
		{
    	    perror("Erreur lors de l'execve");
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << "Parser error: " << e.what() << std::endl;
		return (1);
	}
	return 0;
}
