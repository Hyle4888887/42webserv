
#include "webserv.hpp"
#include "parser/configParser.hpp"

// Program entry point.
int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return (1);
	}
	try
	{
		ConfigParser parser(argv[1]);
		std::cout << "Configuration parsed successfully!" << std::endl;

		const Config& config = parser.getConfig();

		signal(SIGPIPE, SIG_IGN);

		Server	server(config);
		if (config.servers.empty())
		{
			std::cerr << "No server blocks found." << std::endl;
			return (1);
		}

		std::vector<int> ports;
		for (std::size_t i = 0; i < config.servers.size(); ++i)
		{
			int port = config.servers[i].port;
			bool seen = false;
			for (std::size_t j = 0; j < ports.size(); ++j)
			{
				if (ports[j] == port)
				{
					seen = true;
					break;
				}
			}
			if (!seen)
			{
				if (!server.addListener(port))
					return 1;
				ports.push_back(port);
			}
		}

		server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Parser error: " << e.what() << std::endl;
		return (1);
	}
	return 0;
}
