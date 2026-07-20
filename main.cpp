/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbores <mbores@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 12:51:45 by bozil             #+#    #+#             */
/*   Updated: 2026/07/20 13:59:53 by mbores           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "parser/configParser.hpp"

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

		Server	server;

		if (!server.addListener(8080))
			return 1;
		if (!server.addListener(8081))
			return 1;

		server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Parser error: " << e.what() << std::endl;
		return (1);
	}
	return 0;
}
