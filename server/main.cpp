/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 12:51:45 by bozil             #+#    #+#             */
/*   Updated: 2026/06/04 11:03:54 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <csignal>

int	main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	signal(SIGPIPE, SIG_IGN);

	Server	server;

	if (!server.addListener(8080))
		return 1;
	if (!server.addListener(4242))
		return 1;

	server.run();
	return 0;
}
