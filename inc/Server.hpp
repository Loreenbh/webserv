#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/epoll.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <fcntl.h>

# include <map>
# include <sstream>
# include <cstdio>
# include <fstream>

# include "Client.hpp"
# include "ServerConfig.hpp"

# define SERVER_SOFTWARE "Webserv0.1"
# define GATEWAY_INTERFACE "CGI/1.1"
# define SERVER_PROTOCOL "HTTP/1.1"
# define REDIRECT_STATUS "200"

class Client;

class Server
{
	public:
		Server(void);
		Server(ServerConfig *config);
		~Server();
		bool init(void);
		void print(void);
		bool bindSocket(int maxClient);
		bool newClient(void);
		void eraseClient(int clientFd);
		void initEnv(void);
		void initStatusDescriptions(void);

		// GETTERS
		std::map<std::string, std::string> getEnv(void) const;
		int getFd(void) const;
		std::map<int, Client *> getClients(void) const;
		int getEpollFd(void) const;

		struct epoll_event * getEventFds(void);
		int * getNfds(void);

		ServerConfig *getConfig(void) const;
		std::string getStatusDescription(const std::string& code);

		// SETTERS
		void setEpollFd(int fd);
		void setEventFds(struct epoll_event * eventFds);
		void setNfds(int *nfds);

	private:
		int 								_fd;
		int 								_epollFd;
		int *								_nfds;
		struct epoll_event * 				_eventFds;
		bool 								_running;
		struct sockaddr_in 					_address;
		ServerConfig *						_config;
		std::map<int, Client*>				_clients;
		std::map<std::string, std::string> 	_env;
		std::map<std::string, std::string> 	_statusDescriptions;
		

	class ErrorSocket : public std::exception
	{
		public:
			virtual const char *what() const throw();
	};
	class ErrorBind : public std::exception
	{
		public:
			virtual const char *what() const throw();
	};
};

#endif