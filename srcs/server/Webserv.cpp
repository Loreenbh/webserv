/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 09:28:54 by glions            #+#    #+#             */
/*   Updated: 2025/06/03 10:41:54 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include <fcntl.h>
# include <iostream>
# include <unistd.h>
# include <sys/wait.h>
# include "Webserv.hpp"

/**
 * @brief	Constructor for the Webserv class.
 * @details	This constructor initializes the Webserv instance with an empty
 * 			list of servers and an invalid epoll file descriptor.
 * 			It sets up the necessary data structures for managing multiple servers.
 */

Webserv::Webserv() :
	_servers(),
	_epollFd(-1)
{
}

/**
 * @brief	Destructor for the Webserv class.
 * @details	This destructor cleans up all server instances and closes the epoll file descriptor
 * 			if it is valid. It ensures that all resources are properly released when the Webserv instance
 * 			is destroyed.
 */

Webserv::~Webserv(void)
{
	for (size_t i = 0; i < this->_servers.size(); i++)
		delete this->_servers[i];
	if (this->_epollFd != -1)
		close(this->_epollFd);
	for (size_t i = 0; i < this->_requests.size(); i++)
	{
		delete this->_requests[i].first;
		this->_requests[i].first = NULL;
	}
	
}


/**
 * @brief	Parses the configuration file and initializes servers.
 * @param	file_path	The path to the configuration file.
 * @return	Returns true if parsing and server initialization are successful,
 * 			false otherwise.
 * @details	This method reads the configuration file specified by `file_path`,
 * 			parses its contents, and initializes server instances based on the parsed
 * 			configuration. It uses the `ParseConfig` class to handle the parsing.
 */

bool Webserv::parsing(std::string & file_path)
{
	try
	{
		// paring the configuration file
		ParseConfig parsing(file_path);
		if (!parsing.startParsing())
			return (false);
		file_path.clear();
		// get the parsed server configurations and create Server instances
		std::vector<ServerConfig *> configs = parsing.getConfigs();
		for (size_t i = 0; i < configs.size(); i++)
		{
			// create a new Server instance with the parsed configuration
			ServerConfig *serv = new ServerConfig(*configs[i]);
			this->_servers.push_back(new Server(serv));
		}
		// catch any exceptions that may occur during parsing
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return (false);
	}
	// print servers for debugging
	// std::cout << "Servers parsed successfully." << std::endl;
	// this->printServers();
	return (true);
}

/**
 * @brief	Initializes all servers in the Webserv instance.
 * @return	Returns true if at least one server was successfully initialized,
 * 			false if all servers failed to initialize.
 * @details	This method iterates through the list of servers and attempts to
 * 			initialize each one. If a server fails to initialize or bind its socket,
 * 			it is deleted from the list. The method returns true if at least one server
 * 			was successfully initialized, otherwise it returns false.
 */

bool Webserv::initServers(void)
{
	int error = 0;
	int nbServ = this->_servers.size();

	std::cout << "[Webserv] Initializing servers..." << std::endl;
	// Iterate through each server and attempt to initialize it
	for (std::vector<Server *>::iterator it = this->_servers.begin();
		it != this->_servers.end();)
	{
		if ((*it)->getFd() == -1 && (
				!(*it)->init() ||
				!(*it)->bindSocket(MAX_CLIENTS)))
		{
			// If initialization or binding fails, delete the server
			std::string errorMsg = "[!Server "
				+ toString((*it)->getFd()) + "!] Initialization or binding failed";
			std::cerr << errorMsg << std::endl;
			delete *it;
			error++;
			it = this->_servers.erase(it);
			continue ;
		}
		it++;
	}
	return ((error == nbServ) ? false : true);
}

/**
 * @brief	Prepares the Webserv instance for handling events.
 * @return	Returns true if the epoll file descriptor was created successfully
 * 			and at least one server was added to it, false otherwise.
 * @details	This method creates an epoll file descriptor and adds each server's
 * 			file descriptor to it. If any server fails to be added, it is deleted
 * 			from the list of servers. The method returns true if at least one server
 * 			was successfully added to the epoll instance.
 */

bool Webserv::ready(void)
{
	int nbError = 0;
	int nbServer = this->_servers.size();
	this->_epollFd = epoll_create1(0);
	if (this->_epollFd == -1)
	{
		strerror(errno);
		return (false);
	}
	for (std::vector<Server *>::iterator it = this->_servers.begin();
			it != this->_servers.end();)
	{
		struct epoll_event event = {};
		(*it)->setEpollFd(this->_epollFd);
		(*it)->setEventFds(this->getEventFd());
		(*it)->setNfds(this->getNfds());
		event.events = EPOLLIN;
		event.data.fd = (*it)->getFd();
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD,
				(*it)->getFd(), &event) == -1)
		{
			std::string error = "[!Server "
				+ toString((*it)->getFd()) + "!] epoll_ctl";
			strerror(errno);
			std::cerr << error << std::endl;
			// If epoll_ctl fails, delete the server
			delete *it;
			nbError++;
			it = this->_servers.erase(it);
			continue ;
		}
		// If epoll_ctl succeeds, print server information
		std::cout << "[Server " << (*it)->getFd() << "] started" << std::endl;
		if (it != this->_servers.end())
			it++;
	}
	return ((nbError == nbServer) ? true : true);
}


/**
 * @brief Starts the event loop and handles incoming client connections and requests.
 * 
 * @return true if the server ran without critical errors, false otherwise.
 */
bool Webserv::start(void)
{
	while (true)
	{
		this->_nfds = epoll_wait(this->_epollFd, this->_events, 10, EPOLL_TIMEOUT_MS);

		if (this->_nfds == -1)
		{
			std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
			return false;
		}

		for (int i = 0; i < this->_nfds; i++)
			this->handleEvent(this->_events[i]);
		this->checkRequestTimeouts();
	}

	int status = 0;
	waitpid(-1, &status, 0); // PAS AUTORISE
	return true;
}

/**
 * @brief Handles a single epoll event. Determines whether it's a new client or an existing request.
 * 
 * @param event The epoll event to process.
 * @details This method checks if the event corresponds to a new client connection
 * 			or an existing client request. If it's a new client, it calls the `newClient` method
 * 			to handle the connection.
 */
void Webserv::handleEvent(const epoll_event& event)
{
	struct epoll_event* cgiEvent = NULL;
	int fd = event.data.fd;
	bool isNewClient = false;

	// Check if the event corresponds to a server socket (new connection)
	for (size_t j = 0; j < this->_servers.size(); j++)
	{
		if (this->_servers[j]->getFd() == fd)
		{
			isNewClient = this->_servers[j]->newClient();
			return;
		}
	}
	if (!isNewClient)
	{
		// Handle existing client

		if (event.events & EPOLLIN && this->handleClient(event, false))
			std::cout << "[Client fd " << fd << "] request readed." << std::endl;
		else if (event.events & EPOLLOUT && this->handleClient(event, true))
			std::cout << "[Client fd " << fd << "] response sended." << std::endl;
		else
			cgiEvent = const_cast<epoll_event*>(&event);
	}

	// Process requests
	
	for (std::vector<std::pair<Request*, std::time_t> >::iterator it = this->_requests.begin();
		it != this->_requests.end(); ++it)
	{
		if (it->first->handleRequest(cgiEvent, false))
		{
			if (it->first)
			{
				delete it->first;
				it->first = NULL;
			}
			it = this->_requests.erase(it);
			if (it == this->_requests.end())
				break; // Sort de la boucle si l'itérateur atteint la fin du vecteur
		}
	}
}

/**
 * @brief Checks and cleans up requests that have exceeded the timeout threshold.
 * @details This method iterates through the list of requests and increments their timeout counters.
 *          If a request's timeout counter exceeds the defined TIMEOUT_S, it attempts to handle the request.
 *          If the request is successfully handled, it is deleted from the list; otherwise, it remains in the queue.
 */
void Webserv::checkRequestTimeouts()
{
	// if (!this->_requests.empty())
	// 	std::cout << "Requests size: " << this->_requests.size() << std::endl;
	for (std::vector<std::pair<Request*, std::time_t> >::iterator it = this->_requests.begin();
			it != this->_requests.end(); ++it)
	{
		if (std::time(NULL) - it->second >= TIMEOUT_S)
		{
			std::cerr << "[Request] timeout for URI: " << it->first->getHeaderInfo().uri << std::endl;

			if (it->first->handleRequest(NULL, true))
			{
				delete it->first;
				it = this->_requests.erase(it);
				std::cout << "[Request] deleted due to timeout." << std::endl;
				if (it == this->_requests.end())
					break; // break if the iterator reaches the end of the vector
			}
			else
			{
				std::cout << "[Request] not handled, keeping it in the queue." << std::endl;
			}
		}
	}
}

/**
 * @brief Handles a client connection by reading data, parsing requests, and managing client state.
 * @param clientFd The file descriptor of the client to handle.
 * @return Returns true if the client was handled successfully, false otherwise.
 * @details This method reads data from the client, updates the client's request parser,
 *          and checks if a complete request is ready. If a request is ready, it creates a new
 *          Request object and adds it to the list of requests. If an error occurs or the client disconnects,
 *          it cleans up the client resources.
 */
bool Webserv::handleClient(const epoll_event& event, bool sendMode)
{
	Server* serv = this->getServerByClientFd(event.data.fd);
	if (!serv)
		return (false);

	Client* client = this->getClientFromServer(serv, event.data.fd);
	if (!client)	
		return (false);
	if (sendMode)
	{
		ssize_t bytes_sent = send(client->getFd(), client->getResponse().c_str(), client->getResponse().size(), 0);
		if (bytes_sent < 0) {
			strerror(errno);
			client->disconnect();
			return (false);
		}
		struct epoll_event event = {};
		event.events = EPOLLIN;
		event.data.fd = client->getFd();
		if (epoll_ctl(serv->getEpollFd(), EPOLL_CTL_MOD, client->getFd(), &event) == -1)
		{
			strerror(errno);
			return (false);
		}
	}
	else
	{
		char buffer[BUFFER_SIZE] = {0};

		ssize_t bytesRead = recv(event.data.fd, buffer, sizeof(buffer), 0);
		if (bytesRead == 0)
			return (this->handleClientDisconnect(serv, client->getFd(), client));
		else if (bytesRead < 0)
			return (this->handleClientRecvError(serv, client->getFd(), client));
		std::string tmp(buffer, bytesRead);

		if (!client->getParseReq())
			client->newParseRequest(tmp, bytesRead);
		else
			client->updateParseRequest(tmp, bytesRead);

		int error;
		if ((error = client->requestReady()) == 0)
		{
			Request* req = new Request(client->getParseReq());
			client->deleteParseRequest();

			std::cout << "[Server " << serv->getFd() << "] "
					<< "[Client " << client->getFd() << "] "
					<< "[URI]: " << req->getHeaderInfo().uri << std::endl;
			//req->print();	
			this->_requests.push_back(std::make_pair(req, std::time(NULL)));
			return (true);
		}
		else if (error != -1)
		{
			if (!client->buildErrorPage(error))
			{
				delete client;
				client = NULL;
			}
			client->deleteParseRequest();
			return (false);
		}
	}
    return (true);
}

/**
 * @brief Retrieves the client pointer from a server based on the file descriptor.
 * 
 * @param serv Pointer to the server.
 * @param clientFd The client's file descriptor.
 * @return Pointer to the client, or nullptr if not found.
 */
Client* Webserv::getClientFromServer(Server* serv, int clientFd)
{
    std::map<int, Client*> clients = serv->getClients();
    std::map<int, Client*>::iterator it = clients.find(clientFd);
    if (it == clients.end())
        return NULL;
    return it->second;
}


/**
 * @brief Handles a clean client disconnection (recv == 0).
 *        Cleans up associated requests and deletes the client.
 * 
 * @param serv The server instance.
 * @param clientFd The client file descriptor.
 * @param client The client to disconnect.
 * @return Always returns true (clean disconnect).
 */
bool Webserv::handleClientDisconnect(Server* serv, int clientFd, Client* client)
{
    serv->eraseClient(clientFd);
	if (client)
    {
		for (std::vector<std::pair<Request*, std::time_t> >::iterator it = this->_requests.begin();
				it != this->_requests.end(); it++)
		{
			if (it->first->getClient()->getFd() == client->getFd())
			{
				delete it->first;
				it = this->_requests.erase(it);
				if (it == this->_requests.end())
					break;
			}
		}
        client->disconnect();
        delete client;
        client = NULL;
    }
    return true;
}

/**
 * @brief Handles a receive error from a client socket (recv < 0).
 *        Removes client and cleans up memory.
 * 
 * @param serv The server instance.
 * @param clientFd The client file descriptor.
 * @param client The client to remove.
 * @return false indicating an error occurred.
 */
bool Webserv::handleClientRecvError(Server* serv, int clientFd, Client* client)
{
    std::cerr << "recv() failed on client " << clientFd << ": " << strerror(errno) << std::endl;

    serv->eraseClient(clientFd);

    if (client)
    {
        client->disconnect();
        delete client;
        client = NULL;
    }

    return false;
}

/**
 * @brief Finds the server associated with a given client file descriptor.
 * 
 * @param clientFd The file descriptor of the client.
 * @return Pointer to the Server instance if found, or NULL if not found.
 * @details This method iterates through all servers and checks their client maps
 *          to find the server that contains the specified client file descriptor.
 */
Server *Webserv::getServerByClientFd(int clientFd)
{
	for (size_t i = 0; i < this->_servers.size(); i++)
	{
		std::map<int, Client *> clients = this->_servers[i]->getClients();
		if (clients.find(clientFd) != clients.end())
			return (this->_servers[i]);
	}
	return (NULL);
}

/**
 * @brief Prints the details of all servers in the Webserv instance.
 * @details This method iterates through the list of servers and calls their print method,
 *          displaying their configuration and status information to the console.
 */
void Webserv::printServers(void)
{
	std::cout << "SERVERS : " << std::endl;
	for (size_t i = 0; i < this->_servers.size(); i++)
	{
		this->_servers[i]->print();
		std::cout << std::endl;
	}
}

// Getters for Webserv class
/**
 * @brief Returns the list of servers managed by the Webserv instance.
 * @return A vector containing pointers to all Server instances.
 * @details This method provides access to the internal list of servers,
 *          allowing external code to interact with the servers managed by Webserv.
 */
std::vector<Server *> Webserv::getServers(void) const
{
	return (this->_servers);
}

/**
 * @brief Returns the epoll event file descriptor.
 * @return Pointer to the first epoll_event in the events array.
 * @details This method provides access to the epoll event file descriptor,
 *          which is used for monitoring multiple file descriptors to see if they are ready for I/O operations.
 */
struct epoll_event * Webserv::getEventFd(void)
{
	return (&this->_events[0]);
}

/**
 * @brief Returns a pointer to the number of file descriptors being monitored.
 * @return Pointer to the integer representing the number of file descriptors.
 * @details This method provides access to the internal counter of file descriptors,
 *          which is used to track how many file descriptors are currently being monitored by the epoll instance.
 */
int * Webserv::getNfds(void)
{
	return (&this->_nfds);
}