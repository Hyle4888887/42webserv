#include "lexer.hpp"
#include "configParser.hpp"

int main(int argc, char **argv)
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
        std::cout << config.servers[0].host << std::endl;
        std::cout << config.servers[0].port << std::endl;
        std::cout << config.servers[0].serverName << std::endl;
        std::cout << config.servers[0].clientMaxBodySize << std::endl;
        std::cout << config.servers[0].errorPages.at(404) << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Parser error: " << e.what() << std::endl;
        return (1);
    }

    return (0);
}