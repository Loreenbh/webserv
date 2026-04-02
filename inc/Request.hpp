#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <sys/stat.h> //pour stat
#include <dirent.h>   // Pour opendir, readdir et closedir
#include <cstring>

# include "Server.hpp"
# include "Route.hpp"
# include "ParseRequest.hpp"
# include "CGI.hpp"

# define TRADMETHOD(m)(m==GET?"GET":m==POST?"POST":m==DELETE?"DELETE":"")

class Routes;
class Server;
class Client;
class ParseRequest;

class Request {
	private:
	
	t_header							_headerInfo;
	std::string							_body;
	Route*								_route;
	Server*								_server;
	Client*								_client;
	std::string 						_saveRequest;
	std::string 						_finalPath;

	std::string 						_htmlContent;
	std::string 						_dataPost;
	std::string 						_nameFile;
	bool        						_isDirectory;
	std::map<std::string, std::string>	_dataForm;
	std::map<std::string, std::string>	_typeMime;
	std::map<std::string, std::string>	_env;
	std::string							_fileExt;
	CGI*								_cgi;
	// private methods
	int							handleCGITimeout();
	int							responseProcess();
	int 						handleCGIEvent(struct epoll_event *event, int timeout);
	int							selectMethod(struct epoll_event *event, bool timeout);
	int							checkRequest(struct epoll_event *event, bool timeout);
	bool						checkReturn(Route *route);
	std::string					generateHtmlPage(std::string title, std::string body);
	int							listDir(Route *route);
	int							readFile(std::string &path);
	

public:
		//Constructors
		Request(ParseRequest *parseReq);

		//Destructor
		~Request();

		//Operator Loaded

		//Getters
		std::map<std::string, std::string>			getEnv(void) const;
		const t_header 								&getHeaderInfo(void) const;
		const std::string 							&getFileExt(void) const;
		const std::map<std::string, std::string>	&getTypeMime(void) const;
		Client*								getClient(void) const { return this->_client; }
		//Setter

		//Public Methods
		void						initEnv(void);
		void						print(void);

		bool						handleRequest(struct epoll_event *event, bool timeout);

};
