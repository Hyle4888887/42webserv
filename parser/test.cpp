#include "lexer.hpp"
#include "configParser.hpp"

int main(int argc, char **argv)
{
    // if (argc != 2)
    //     return (1);

    // std::ifstream file(argv[1]);

    // std::stringstream buffer;
    // buffer << file.rdbuf();

    // Lexer lexer(buffer.str());

    // std::vector<Token> tokens = lexer.tokenize();

    // for (size_t i = 0; i < tokens.size(); i++)
    // {
    //     std::cout << tokens[i].type
    //               << " : "
    //               << tokens[i].value
    //               << std::endl;
    // }

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return (1);
    }

    try
    {
        ConfigParser parser(argv[1]);

        std::cout << "Configuration parsed successfully!" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Parser error: " << e.what() << std::endl;
        return (1);
    }

    return (0);
}