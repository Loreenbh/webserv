/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/11 11:04:34 by glions            #+#    #+#             */
/*   Updated: 2025/05/30 11:02:14 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"



ServerConfig::ServerConfig(void) :
	_serverName(),
	_port(-1),
	_clientMaxBody(0),
	_errorPages(),
	_routes()
{
}

ServerConfig::ServerConfig(ServerConfig const &copy):
	_serverName(copy.getServerName()),
	_port(copy.getPort()),
	_clientMaxBody(copy.getClientMaxBody()),
	_routes()
{
	std::map<std::string, Route *> tmp = copy.getRoutes();
	for (std::map<std::string, Route *>::iterator it = tmp.begin();
		it != tmp.end(); it++)
		this->_routes[it->first] = new Route(*(it->second));
	std::map<int, std::string> tmp2 = copy.getErrorPages();
	for(std::map<int, std::string>::iterator it = tmp2.begin();
		it != tmp2.end(); ++it)
		this->_errorPages[it->first] = it->second;
}

ServerConfig::~ServerConfig()
{
	for (std::map<std::string, Route *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++)
		delete it->second;
}

ServerConfig &ServerConfig::operator=(const ServerConfig &copy)
{
	if (this != &copy)
	{
		this->_port = copy.getPort();
		this->_serverName = copy.getServerName();
		this->_clientMaxBody = copy.getClientMaxBody();
		std::map<std::string, Route *> tmp = copy.getRoutes();
		for (std::map<std::string, Route *>::iterator it = tmp.begin();
			it != tmp.end(); it++)
			this->_routes[it->first] = new Route(*(it->second));
		std::map<int, std::string> tmp2 = copy.getErrorPages();
		for(std::map<int, std::string>::iterator it = tmp2.begin();
			it != tmp2.end(); ++it)
			this->_errorPages[it->first] = it->second;
	}
	return (*this);
}

void ServerConfig::print(void)
{
	std::cout << "Configs : " << std::endl;
	std::cout << "Server name : " << this->_serverName << std::endl;
	std::cout << "Port : " << this->_port << std::endl;
	std::cout << "Client max body : " << this->_clientMaxBody << std::endl;
	std::cout << "Errors pages : " << std::endl;
	for (std::map<int, std::string>::iterator it = this->_errorPages.begin();
		it != this->_errorPages.end(); ++it)
		std::cout << "Code : " << it->first << " -> " << it->second << std::endl;
	std::cout << "Routes : " << std::endl;
	std::cout << "size : " << this->_routes.size() << std::endl;
	for (std::map<std::string, Route *>::iterator it = this->_routes.begin();
		it != this->_routes.end(); ++it)
	{
		it->second->print();
		std::cout << std::endl;
	}
}

// SETTERS
void ServerConfig::setServerName(std::string name)
{
	this->_serverName = name;
}

void ServerConfig::setServerName(std::vector<std::string> args)
{
	if (this->_serverName != "")
		throw ServerConfig::ErrorDuplicate();
	if (args.size() > 2)
	{
		throw ServerConfig::ErrorToManyArgs();
	}
	else if (args.size() < 2)
	{
		throw ServerConfig::ErrorNotEnoughArgs();
	}
	this->_serverName = args[1];
}

void ServerConfig::setPort(std::vector<std::string> args)
{
	if (this->_port != -1)
		throw ServerConfig::ErrorDuplicate();
	else if (args.size() > 2)
		throw ServerConfig::ErrorToManyArgs();
	else if (args.size() < 2)
		throw ServerConfig::ErrorNotEnoughArgs();
	for (size_t i = 0; i < args[1].size(); i++)
	{
		if (!isdigit(args[1].at(i)))
			throw ServerConfig::ErrorNotValidArgs();
	}
	std::istringstream iss(args[1]);
	int num;
	if (!(iss >> num))
		throw ServerConfig::ErrorNotValidArgs();
	this->_port = num;
}

void ServerConfig::setPort(int port)
{
	this->_port = port;
}

void ServerConfig::addErrorPage(std::vector<std::string> args)
{
	/*
		Verifier si le code d'erreur est valide
		Verifier si le path est valide
	*/
	if (args.size() > 3)
		throw ServerConfig::ErrorToManyArgs();
	else if (args.size() < 3)
		throw ServerConfig::ErrorNotEnoughArgs();
	std::istringstream iss(args[1]);
	int num;
	if (!(iss >> num))
		throw ServerConfig::ErrorNotValidArgs();
	if (this->_errorPages.find(num) != this->_errorPages.end())
		throw ServerConfig::ErrorErrorPageAlreadyIn();
	this->_errorPages[num] = args[2];
}

void ServerConfig::setClientMaxBody(std::vector<std::string> args)
{
	if (this->_clientMaxBody != 0)
		throw ServerConfig::ErrorDuplicate();
	if (args.size() > 2)
		throw ServerConfig::ErrorToManyArgs();
	else if (args.size() < 2)
		throw ServerConfig::ErrorNotEnoughArgs();
	if (args[1].size() == 1 && !isdigit(args[1].at(0)))
		throw ServerConfig::ErrorNotValidArgs();
	for (size_t i = 0; i < args[1].size(); i++)
	{
		if (!isdigit(args[1].at(i)) && ((args[1].at(i) != 'K' && args[1].at(i) != 'M' &&
			args[1].at(i) != 'G') || i != args[1].size() - 1))
			throw ServerConfig::ErrorNotValidArgs();
	}
	char u = args[1].at(args[1].size() - 1);
	if (!isdigit(u))
		args[1].resize(args[1].size() - 1);
	std::istringstream iss(args[1]);
	size_t num;
	if (!(iss >> num))
		throw ServerConfig::ErrorNotValidArgs();
	if (!isdigit(u))
	{
		num = 1;
		switch (u)
		{
			case 'G' :
				num = num * 1024 * 1024 * 1024;
				break ;
			case 'M' :
				num = num * 1024 * 1024;
				break ;
			case 'K' :
				num = num * 1024;
				break;
			default :
				std::cerr << "UNITE PAS TROUVE, PAS NORMAL" << std::endl;
		}
	}
	//std::cout << "client_max_body ->" << num << std::endl;
	this->_clientMaxBody = num;
}

void ServerConfig::addRoute(Route *route)
{
	this->_routes.insert(std::make_pair(route->getPath(), route));
}

// GETTERS
std::string ServerConfig::getServerName(void) const
{
	return (this->_serverName);
}

size_t ServerConfig::getClientMaxBody(void) const
{
	return (this->_clientMaxBody);
}

int ServerConfig::getPort(void) const
{
	return (this->_port);
}
std::map<std::string, Route *> ServerConfig::getRoutes(void) const
{
	return (this->_routes);
}

std::map<int, std::string> ServerConfig::getErrorPages(void) const
{
	return (this->_errorPages);
}

// EXCEPTIONS
const char *ServerConfig::ErrorNotEnoughArgs::what() const throw()
{
	return ("not enough args");
}

const char *ServerConfig::ErrorToManyArgs::what() const throw()
{
	return ("to much args");
}

const char *ServerConfig::ErrorNotValidArgs::what() const throw()
{
	return ("not valid args");
}

const char *ServerConfig::ErrorErrorPageAlreadyIn::what() const throw()
{
	return ("error pages already present");
}

const char *ServerConfig::ErrorDuplicate::what() const throw()
{
	return ("duplicate rules on file");
}