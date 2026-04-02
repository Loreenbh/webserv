/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 11:23:17 by tissad            #+#    #+#             */
/*   Updated: 2025/06/02 11:08:17 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef CGI_HPP
# define CGI_HPP

# include <sys/epoll.h>
# include <fcntl.h>
# include <unistd.h>

# include <string>
# include <vector>
# include <iostream>
# include <cstring>
# include <map>
# include <ctime>

# define BUFFER_SIZE 1024
# define CGI_PATH "/usr/bin/php-cgi"
# define ERROR_500 500
# define ERROR_504 504
# define ERROR_400 400

# define EPOLL_TIMEOUT_MS 50
# define TIMEOUT_S 5
# define MAX_EVENTS 3
class CGI
{
	private:
		// private attributes
		std::map<std::string, std::string>	_envMap;
		std::string 						_path;
		int									_epollFd;
		

		std::string							_body;
		char**								_env;
		char**								_args;
		int									_error;
		pid_t								_pid;
		// int									_status;
		std::string							_result;
		int									_cgi_timer; // timer fd for cgi timeout ms
		std::vector<std::string>			_logs;
	
		// pipes
		int									_stdin_pipe[2];  // 0: read, 1: write
		int									_stdout_pipe[2]; // 0: read, 1: write


	
		
		
		// private methods
		bool		cgiBuildArgs(std::string query);
		bool		cgiBuildEnv(void);
		bool		cgiBuild(void);
		bool		cgiExec(void);
		bool 		epollAddFd(int fd, int event);
		void		memoryCleanUp(void);
		
		//bool 		cgiWrite(void);
		bool		cgiCreatePipe(void);
		bool		cgiRedirectChild(void);
		void		cgiAddLog(std::string log, int line);
		
	public:
		// default constructor
		CGI(void);
		CGI(std::string path, std::map<std::string, std::string> env, int epollFd);
		CGI(std::string path, std::map<std::string, std::string> env, int epollFd, std::string body);
		~CGI();
		bool cgiStart(void);
		void cgiDisplayLog(void);
		std::string getResult(void) const;
		
		int			cgiRead(struct epoll_event *event, int timeout);
		int			cgiWrite(struct epoll_event *event, int timeout);
		
		int getStdinPipe(void) const { return this->_stdin_pipe[1]; }
		int getStdoutPipe(void) const { return this->_stdout_pipe[0]; }

		int getError(void) const { return this->_error; }
		int getPid(void) const { return this->_pid; }
		
};
# endif