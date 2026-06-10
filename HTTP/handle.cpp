/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bozil <bozil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 10:34:11 by bozil             #+#    #+#             */
/*   Updated: 2026/06/10 12:53:01 by bozil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP.cpp"

// sert a resoudre le path URL en path systeme de fichier en fonction de la config de la route
std::string Response::handleGET(const Request &req, const RouteConfig &route, const ServerConfig &config)
{
}

// sert a gerer les requetes POST (upload de fichier)
std::string Response::handlePOST(const Request &req, const RouteConfig &route, const ServerConfig &config)
{
}

// sert a gerer les requetes DELETE (suppression de fichier)
std::string Response::handleDELETE(const Request &req, const RouteConfig &route, const ServerConfig &config)
{
}