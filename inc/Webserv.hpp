/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glions <glions@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 09:24:10 by glions            #+#    #+#             */
/*   Updated: 2025/05/31 20:24:36 by glions           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "ParseConfig.hpp"
# include "Server.hpp"
# include "Request.hpp"

# include <ctime>

# define MAX_CLIENTS 100
# define BUFFER_SIZE 1024

typedef struct s_req
{
	std::string header;
	char *body;
}   t_req;

class Webserv
{
	public:
		Webserv(void);
		~Webserv();
		// Webserv &operator=(Webserv const &copy);
		void printServers(void);
		bool parsing(std::string &file_path);
		bool initServers(void);
		bool ready(void);
		bool start(void);
		// Getters
		std::vector<Server *>	getServers() const;
		struct epoll_event *	getEventFd(void);
		int *					getNfds(void);

	private:
		std::vector<Server *> _servers;
		
		int _epollFd;
		int _nfds;
		struct epoll_event _events[10];

		std::vector<char> _body;
		std::vector<std::pair<Request *, std::time_t> > _requests;

		// Private methods
		void    handleEvent(const epoll_event& event);
		void    checkRequestTimeouts();
		bool 	handleClient(const epoll_event& event, bool sendMode);
		bool    handleClientDisconnect(Server* serv, int clientFd, Client* client);
		bool    handleClientRecvError(Server* serv, int clientFd, Client* client);
		Client* getClientFromServer(Server* serv, int clientFd);
		Server* getServerByClientFd(int fdClient);
};

#endif