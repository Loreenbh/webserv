/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 23:39:38 by tissad            #+#    #+#             */
/*   Updated: 2025/06/03 10:42:29 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"


/**
 * @brief	Constructor for the Server class.
 * @details	This constructor initializes the Server instance with default values.
 * 			It sets the file descriptor to -1, running status to false, and config to NULL.
 * @note	Use the init() method to properly set up the server before using it.
 * @param	config	A pointer to a ServerConfig object that contains server configuration.
 * @throw	None
 * @note	This constructor is used to create a server instance without any specific configuration.
 *
 */
Server::Server(void) :
    _fd(-1),
    _running(false),
    _config(NULL)
{
    this->initStatusDescriptions();   
}

/**
 * @brief	Constructor for the Server class with configuration.
 * @details	This constructor initializes the Server instance with a given configuration.
 * 			It sets the file descriptor to -1, running status to false, and config to the provided ServerConfig object.
 * @param	config	A pointer to a ServerConfig object that contains server configuration.
 * @throw	None
 * @note	This constructor is used to create a server instance with a specific configuration.
 */
Server::Server(ServerConfig *config) :
    _fd(-1),
    _running(false),
    _config(config)
{
    this->initStatusDescriptions();   
}

/**
 * @brief	Destructor for the Server class.
 * @details	This destructor cleans up resources used by the Server instance, including closing the file descriptor,
 * 			deleting clients, and deleting the configuration object if it exists.
 * @note	This destructor is called when the Server instance goes out of scope or is deleted.
 */
Server::~Server(void)
{
    if (this->_clients.size() > 0)
    {
        for (std::map<int, Client*>::iterator it = this->_clients.begin();
            it != this->_clients.end(); ++it)
        {
            std::cout << "[Server " << this->_fd << "] [Client " << it->first << "] destroyed" << std::endl;
            if (it->second)
            {
                    it->second->disconnect();
                    delete it->second;
                    it->second = NULL;
            }
        }
        this->_clients.erase(this->_clients.begin(), this->_clients.end());
    }
    if (this->_config)
    {
        delete this->_config;
        this->_config = NULL;
    }
    if (this->_fd != -1)
        close(this->_fd);
    std::cout << "[Server " << this->_fd << "] destroyed" << std::endl;
}

/**
 * @brief	Initializes the status descriptions for the server.
 * @details	This method populates the _statusDescriptions map with HTTP status codes and their corresponding descriptions.
 * 			It is called in the constructor to ensure that the server has a predefined set of status codes available.
 * @note	This method is called automatically when a Server instance is created.
 */
void Server::initStatusDescriptions(void)
{
    this->_statusDescriptions["200"] = "OK";
    this->_statusDescriptions["301"] = "Moved Permanently";
    this->_statusDescriptions["502"] = "Bad Gateway";
    this->_statusDescriptions["508"] = "Loop Detected";
    this->_statusDescriptions["400"] = "Bad Request";
    this->_statusDescriptions["403"] = "Forbidden";
    this->_statusDescriptions["404"] = "Not Found";
    this->_statusDescriptions["405"] = "Not Allowed";
    this->_statusDescriptions["408"] = "Request Timeout";
    this->_statusDescriptions["411"] = "Length Required";
    this->_statusDescriptions["413"] = "Payload Too Large";
    this->_statusDescriptions["414"] = "URI Too Long";
    this->_statusDescriptions["415"] = "Unsupported Media Type";
    this->_statusDescriptions["500"] = "Internal Server Error";
    this->_statusDescriptions["501"] = "Not Implemented";
    this->_statusDescriptions["505"] = "HTTP Version Not Supported";
    this->_statusDescriptions["511"] = "Network Authentication Required";
    this->_statusDescriptions["503"] = "Service Unavailable";
    this->_statusDescriptions["504"] = "Gateway Timeout";
    this->_statusDescriptions["429"] = "Too Many Requests";
    this->_statusDescriptions["418"] = "I'm a teapot";

}

/**
 * @brief	Initializes the server socket.
 * @details	This method creates a socket, sets it to non-blocking mode, and initializes the server environment variables.
 * 			It returns true if the socket is successfully created, false otherwise.
 * @return	Returns true if the socket is initialized successfully, false otherwise.
 * @throw	ErrorSocket If there is an error creating the socket.
 */
bool Server::init(void)
{
     int reuse = 1;
    this->_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    if (fcntl(this->_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::string error = "[!Server " + toString(this->getFd()) + "!] fcntl";
        strerror(errno);
        return (false);
    }
    if (this->_fd == -1)
    {
        std::string error = "[!Server " + toString(this->getFd()) + "!] socket";
        strerror(errno);
        return (false);
    }
   

    std::cout << "[Server " << toString(this->getFd()) <<
        "] socket initialized" << std::endl;
    this->initEnv();
    return (true);
}

/**
 * @brief	Initializes the server environment variables.
 * @details	This method sets up the environment variables that will be used by the server,
 * 			such as GATEWAY_INTERFACE, SERVER_SOFTWARE, SERVER_PROTOCOL, SERVER_NAME, SERVER_PORT, and REDIRECT_STATUS.
 * @note	This method is called automatically when a Server instance is created.
 */
void Server::initEnv(void)
{
    this->_env["GATEWAY_INTERFACE"] = GATEWAY_INTERFACE;
    this->_env["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
    this->_env["SERVER_PROTOCOL"] = SERVER_PROTOCOL;
    this->_env["SERVER_NAME"] = this->_config->getServerName();
    this->_env["SERVER_PORT"] = toString(this->_config->getPort());
    this->_env["REDIRECT_STATUS"] = "200";
}

/**
 * @brief	Binds the server socket to the specified port and starts listening for incoming connections.
 * @details	This method binds the server socket to the address specified in the server configuration and starts listening for incoming connections.
 * 			It returns true if the binding and listening are successful, false otherwise.
 * @param	maxClient	The maximum number of clients that can be queued for connection.
 * @return	Returns true if the socket is bound and listening successfully, false otherwise.
 * @throw	ErrorBind If there is an error binding the socket or starting to listen.
 */
bool Server::bindSocket(int maxClient)
{
    std::cerr << "[Server " << this->_fd << "] Binding to port "
        << this->_config->getPort() << std::endl;
    this->_address.sin_family = AF_INET;
    this->_address.sin_addr.s_addr = INADDR_ANY;
    this->_address.sin_port = htons(this->_config->getPort());
    if (bind(this->_fd,
            reinterpret_cast<struct sockaddr*>(&this->_address),
            sizeof(this->_address)) == -1)
    {
        std::string error = "[!Server " + toString(this->getFd()) + "!] bind";
        strerror(errno);
        std::cerr << error << std::endl;
        return (false);
    }
    if (listen(this->_fd, maxClient) == -1)
    {
        std::string error = "[!Server " + toString(this->getFd()) + "!] listen";
        std::cerr << error << std::endl;
        strerror(errno);
        return (false);
    }
    std::cout << "[Server " << this->_fd << "] Listening on port "
        << ntohs(this->_address.sin_port) << std::endl;
    return (true);
}

/**
 * @brief	Accepts a new client connection.
 * @details	This method accepts a new client connection and adds it to the server's client map.
 * 			It sets the client socket to non-blocking mode and returns true if the client is successfully added, false otherwise.
 * @return	Returns true if a new client is successfully accepted, false otherwise.
 * @throw	ErrorClient If there is an error accepting the client connection.
 */
bool Server::newClient(void)
{
    Client *client = new Client(*this);
    socklen_t addr_len = sizeof(struct sockaddr_in);
    
    struct sockaddr *client_addr = reinterpret_cast<struct sockaddr *>(&(client->getAddr()));
    int client_fd = accept(this->_fd, client_addr, &addr_len);
    if (client_fd == -1)
    {
        strerror(errno);
        if (client)
        {
            delete client;
            client = NULL;
        }
        return (false);
    } 
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::string error = "[!Server " + toString(this->getFd()) + "!] fcntl";
        strerror(errno);
        if (client)
        {
            delete client;
            client = NULL;
        }
        return (false);
    }
    try
    {
        client->setFd(client_fd); 
        this->_clients.insert(std::make_pair(client_fd, client));
    } catch (std::exception &e)
    {
        if (client_fd != -1)
            close(client_fd);
        if (client)
        {
            delete client;
            client = NULL;
        }
        std::cerr << e.what() << std::endl;
        return (false);
    }
    std::cout << "[Server " << this->_fd << "] [Client " << client_fd  << "] connected" << std::endl;
    return (true);
}

/**
 * @brief	Erases a client from the server's client map.
 * @details	This method removes a client from the server's client map based on the provided file descriptor.
 * 			If the client is not found, it prints an error message.
 * @param	clientFd	The file descriptor of the client to be removed.
 * @note	This method is used to clean up resources when a client disconnects.
 */
void Server::eraseClient(int clientFd)
{
    if (this->_clients.find(clientFd) != this->_clients.end())
        this->_clients.erase(clientFd);
    else
        std::cerr << "eraseClient : Client unknown" << std::endl;
}

/**
 * @brief	Prints the server's file descriptor and running status.
 * @details	This method outputs the server's file descriptor and whether it is currently running.
 * 			It also prints the server configuration details.
 * @note	This method is used for debugging purposes to check the server's status.
 */
void Server::print(void)
{
    std::cout << "FD : " << this->_fd << std::endl;
    std::cout << "Running : " << this->_running << std::endl;
    this->_config->print();
}

// Setters
void Server::setEpollFd(int fd)
{
    this->_epollFd = fd;
}
void Server::setEventFds(struct epoll_event * eventFds)
{
    this->_eventFds = eventFds;
}

void Server::setNfds(int * nfds)
{
    this->_nfds = nfds;
}

// Getters
int Server::getFd(void) const
{
    return (this->_fd);
}

int Server::getEpollFd(void) const
{
    return (this->_epollFd);
}

int *Server::getNfds(void)
{
    return (this->_nfds);
}

struct epoll_event * Server::getEventFds(void)
{
    return (this->_eventFds);
}


ServerConfig *Server::getConfig(void) const
{
    return (this->_config);
}

std::map<std::string, std::string> Server::getEnv(void) const
{
    return (this->_env);
}

std::map<int, Client *> Server::getClients(void) const
{
    return (this->_clients);
}

/**
 * @brief	Retrieves the description for a given HTTP status code.
 * @details	This method looks up the provided status code in the server's status descriptions map
 * 			and returns the corresponding description. If the code is not found, it returns "Unknown Status".
 * @param	code	The HTTP status code as a string.
 * @return	Returns the description of the status code, or "Unknown Status" if the code is not found.
 */
std::string Server::getStatusDescription(const std::string& code) {
    

    if (this->_statusDescriptions.find(code) != this->_statusDescriptions.end()) {
        return this->_statusDescriptions[code];
    } else {
        return "Unknown Status";
    }
}

// EXCEPTIONS
const char *Server::ErrorSocket::what() const throw()
{
    return ("Error creation socket on Server constructor");
}

const char *Server::ErrorBind::what() const throw()
{
    return ("Error on bindSocket method");
}
