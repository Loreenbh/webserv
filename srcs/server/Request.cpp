/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 23:45:48 by tissad            #+#    #+#             */
/*   Updated: 2025/06/02 14:38:53 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include "Request.hpp"

/**
 * @brief	Constructor for the Request class.
 * @details	This constructor initializes a Request object using a ParseRequest object.
 * 			It sets up the header information, body, route, server, client, and other attributes.
 * @param	parseReq	A pointer to a ParseRequest object containing the request data.
 * @return	None
 * @note	This constructor is used to create a Request object from a parsed request.
 * @throw	None
 */
Request::Request(ParseRequest *parseReq) :
	_headerInfo(parseReq->getHeaderInfo()),
	_body(parseReq->getBody()),
	_route(parseReq->getRoute()),
	_server(parseReq->getServer()),
	_client(parseReq->getClient()),
	_saveRequest(parseReq->getSaveRequest()),
	_finalPath(parseReq->getFinalPath()),
	_htmlContent(),
	_dataPost(),
	_nameFile(),
	_isDirectory(parseReq->getIsDirectory()),
	_cgi(NULL)
{
	this->_typeMime[".html"] = "text/html";

	this->_typeMime[""]     = "text/plain";
	this->_typeMime[".txt"] = "text/plain";
	this->_typeMime[".csv"] = "text/csv";
	this->_typeMime[".php"] = "text/html";

	this->_typeMime[".css"] = "text/css";
	this->_typeMime[".scss"] = "text/x-scss";
	this->_typeMime[".less"] = "text/x-less";
	this->_typeMime[".js"] = "text/javascript";
	this->_typeMime[".js.map"] = "application/json";
	this->_typeMime[".map"] = "application/json";
	this->_typeMime[".wasm"] = "application/wasm";


	this->_typeMime[".json"] = "application/json";
	this->_typeMime[".xml"] = "application/xml";

	this->_typeMime[".jpg"] = "image/jpeg";
	this->_typeMime[".jpeg"] = "image/jpeg";
	this->_typeMime[".png"] = "image/png";
	this->_typeMime[".gif"] = "image/jpeg";
	this->_typeMime[".svg"] = "image/svg+xml";
	this->_typeMime[".webp"] = "image/webp";
	this->_typeMime[".ico"] = "image/x-icon";

	this->_typeMime[".mp4"] = "video/mp4";
	this->_typeMime[".webm"] = "video/webm";
	this->_typeMime[".ogv"] = "video/ogg";

	this->_typeMime[".mp3"] = "audio/mpeg";
	this->_typeMime[".ogg"] = "audio/ogg";
	this->_typeMime[".wav"] = "audio/wav";

	this->_typeMime[".pdf"] = "application/pdf";
	this->_typeMime[".zip"] = "application/zip";
	this->_typeMime[".tar"] = "application/x-tar";
	this->_typeMime[".gz"] = "application/gzip";
	this->_typeMime[".bz2"] = "application/x-bzip2";
}

/**
 * @brief	Destructor for the Request class.
 * @details	This destructor cleans up resources used by the Request instance, including deleting the CGI object if it exists.
 * @note	This destructor is called when the Request instance goes out of scope or is deleted.
 */
Request::~Request(){
	if (this->_cgi != NULL)
	{
		delete this->_cgi;
		this->_cgi = NULL;
	}

}

/**
 * @brief	Handles the request by processing the route and sending the appropriate response.
 * @details	This method checks if a route exists, handles redirects, and processes the request based on the HTTP method.
 * 			It sends the response back to the client or an error page if necessary.
 * @param	event	A pointer to an epoll_event structure for handling events.
 * @param	timeout	The timeout value for epoll_wait.
 * @return	True if the request was handled successfully, false otherwise.
 */
bool    Request::handleRequest(struct epoll_event *event, bool timeout){
	if (this->_route == NULL)
	{
		std::cerr << "route is NULL, cannot handle request." << std::endl;
		return (false);
	}
	if (this->_route != NULL)
	{
		if (this->_client && routeExists(this->_client->getHistoric(), this->_route->getPath())){
			this->_client->clearHistoric();
			if (!this->_client->buildErrorPage(508)){
				delete this->_client;
				this->_client = NULL;
			}
			return (true);
		}
		if (this->_client && this->checkReturn(this->_route)){
			// std::cerr << "Redirecting to: " << this->_route->getRedir().path << std::endl;
			if (!this->_client->buildResponse("", 1, 301, this))
			{
				if (this->_client)
				{
					this->_client->clearHistoric();
					this->_client->disconnect();
					delete this->_client;
					this->_client = NULL;
				}
			}
			// if (!this->_client->sendResponse("", 1, 301, this)){
			// 	delete this->_client;
			// 	this->_client = NULL;
			// }
			return (true);
		}
		int ret = this->selectMethod(event, timeout);
		if (ret == 200)
		{
			if (!this->_client->buildResponse(this->_htmlContent, 0, ret, this))
			{
				if (this->_client)
				{
					this->_client->clearHistoric();
					this->_client->disconnect();
					delete this->_client;
					this->_client = NULL;
				}
			}
			// if (!this->_client->sendResponse(this->_htmlContent, 0, ret, this))
			// {
			// 	if (this->_client)
			// 	{
			// 		this->_client->clearHistoric();
			// 		this->_client->disconnect();
			// 		delete this->_client;
			// 		this->_client = NULL;
			// 	}
			// }
			return (true);
		}
		else if (ret == -1)
		{
			return (false);
		}
		else
		{
			if (this->_client != NULL && !this->_client->buildErrorPage(ret))
			{
				std::cerr << "Error buildding error page" << std::endl;
				if (this->_client)
				{
					this->_client->clearHistoric();
					this->_client->disconnect();
					delete this->_client;
					this->_client = NULL;
				}
			}
			return (true);
		}
	}
	else
	{
		this->_client->clearHistoric();
		if (!this->_client->buildErrorPage(400)){
			delete this->_client;
			this->_client = NULL;
		}
		return (true);
	}
}

/**
 * @brief	Prints the request information to the console.
 * @details	This method outputs the request's header information, route, and body content to the console for debugging purposes.
 * @note	This method is used for debugging and logging the request details.
 */
void Request::print(void)
{
	std::cout << "Method             : " << this->_headerInfo.method << std::endl;
	std::cout << "Url                : " << this->_headerInfo.url << std::endl;
	std::cout << "Version            : " << this->_headerInfo.version << std::endl;
	std::cout << "Host               : " << this->_headerInfo.hostName << std::endl;
	std::cout << "User_Agent         : " << this->_headerInfo.agent << std::endl;
	std::cout << "Accept             : " << this->_headerInfo.accept << std::endl;
	std::cout << "AcceptLanguage     : " << this->_headerInfo.acceptLanguage << std::endl;
	std::cout << "AcceptEncoding     : " << this->_headerInfo.acceptEncoding << std::endl;
	std::cout << "Connection         : " << this->_headerInfo.connection << std::endl;
	if (this->_headerInfo.contentLength != -1)
		std::cout << "Content-Length     : " << this->_headerInfo.contentLength << std::endl;
	if (this->_headerInfo.transferEncoding)
		std::cout << "Transfert-Encoding : true" << std::endl;
	if (!   this->_headerInfo.contentType.empty())
		std::cout << "ContentType        : " << this->_headerInfo.contentType << std::endl;
	if (!this->_headerInfo.boundary.empty())
		std::cout << "Boundary           : " << this->_headerInfo.boundary << std::endl;
	std::cout << "Route :" << std::endl;
	this->_route->print();
	std::cout << std::endl;
	std::cout << "----- BODY -----" << std::endl;
	std::cout << this->_body << std::endl;
	std::cout << "----------------" << std::endl;
}

/**
 * @brief	Initializes the environment variables for the request.
 * @details	This method sets up the environment variables based on the request header information and server configuration.
 * 			It prepares the environment for CGI execution if applicable.
 * @note	This method is called before processing CGI requests to ensure the environment is correctly set up.
 */
void Request::initEnv(void)
{
	this->_env = this->_server->getEnv();
	this->_env["REQUEST_METHOD"] = TRADMETHOD(this->_headerInfo.method);

	this->_env["PATH_INFO"] = this->_headerInfo.pathInfo;

	this->_env["QUERY_STRING"] = this->_headerInfo.queryString;

	this->_env["CONTENT_TYPE"] = this->_headerInfo.contentType;
	this->_headerInfo.contentLength = this->_headerInfo.contentLength == -1 ? 0 : this->_headerInfo.contentLength;
	this->_env["CONTENT_LENGTH"] = toString(this->_headerInfo.contentLength);
	
	this->_env["SCRIPT_FILENAME"] = this->_finalPath;

	this->_env["HTTP_USER_AGENT"] = this->_headerInfo.agent;
	this->_env["HTTP_ACCEPT"] = this->_headerInfo.accept;
	this->_env["HTTP_ACCEPT_LANGUAGE"] = this->_headerInfo.acceptLanguage;
	this->_env["HTTP_ACCEPT_ENCODING"] = this->_headerInfo.acceptEncoding;

	this->_env["REQUEST_URI"] = this->_headerInfo.url;

	this->_env["REMOTE_ADDR"] = this->_client->getAddr().sin_addr.s_addr;
	this->_env["REMOTE_PORT"] = toString(this->_client->getAddr().sin_port);
}

/**
 * @brief	Generates an HTML page with the specified title and body content.
 * @details	This method creates a complete HTML document with a given title and body content.
 * 			It returns the generated HTML as a string.
 * @param	title	The title of the HTML page.
 * @param	body	The body content of the HTML page.
 * @return	A string containing the complete HTML document.
 */
std::string Request::generateHtmlPage(std::string title, std::string body) {

	std::string htmlContent;
	htmlContent += "<!DOCTYPE html>\n";
	htmlContent += "<html lang=\"en\">\n";
	htmlContent += "<head>\n";
	htmlContent += "<meta charset=\"UTF-8\">\n";
	htmlContent += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	htmlContent += "<title>" + title + "</title>\n";
	htmlContent += "</head>\n";
	htmlContent += "<body>\n";
	htmlContent += body;
	htmlContent += "</body>\n";
	htmlContent += "</html>\n";
	return (htmlContent);
}


/**
 * @brief	Reads a file from the specified path and sets the HTML content.
 * @details	This method attempts to read a file from the given path and sets the HTML content based on the file's content.
 * 			If the file is not found, it returns a 404 error.
 * @param	path	The path to the file to be read.
 * @return	Returns 200 if the file is read successfully, or 404 if the file is not found.
 */
int Request::listDir(Route *route)
{
	DIR *dir;
	struct dirent *entry;
	std::string html;
	std::vector<struct dirent *> entrys;
	dir = opendir(this->_finalPath.c_str());
	if (!dir)
	{
		std::cerr << "ERROR open directory : " << this->_finalPath<< std::endl;
		return (500);
	}
	html = "<h1>Directory listing :</h1><ul>";
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		std::string routeName = route->getPath();
		if (routeName.at(routeName.size() - 1) != '/')
			routeName.insert(routeName.size(), "/");
		if (name == ".." && this->_headerInfo.url == routeName)
			continue ;
		std::string link;
		if (this->_headerInfo.url.at(this->_headerInfo.url.size() - 1) != '/')
			link = this->_headerInfo.url + "/" + name;
		else
			link = this->_headerInfo.url + name;
		std::string tmp = "<li><a href=\"" + link + "\">" + name + "</a></li>";
		html += tmp;
	}
	html += "</ul>";
	closedir(dir);
	this->_htmlContent = this->generateHtmlPage("Directory listing", html);
	return (200);
}

/**
 * @brief	Selects the appropriate method for processing the request based on the HTTP method.
 * @details	This method checks the HTTP method of the request and calls the appropriate processing function.
 * 			It handles GET, POST, and DELETE methods, and returns an error code if the method is not supported.
 * @param	route	A pointer to the Route object associated with the request.
 * @param	event	A pointer to an epoll_event structure for handling events.
 * @param	timeout	The timeout value for epoll_wait.
 * @return	Returns 200 if the request is processed successfully, or an error code if there is an issue.
 */

int	Request::selectMethod(struct epoll_event *event, bool timeout){
	if (this->_headerInfo.method == POST )
	{

		if (this->_route->getCgiAccept() && isCGI(this->_finalPath))
		{
			return (this->checkRequest(event, timeout));
		}
		else
		{
			std::cerr << "ERROR POST" << std::endl;
			return (405);
		}
	}
	else if (this->_headerInfo.method == GET)
	{
		return(this->checkRequest(event, timeout));
	}
	else if (this->_headerInfo.method == DELETE)
	{
		if (this->_route->getCgiAccept() && isCGI(this->_finalPath))
		{
			return(this->checkRequest(event, timeout));
		}
		else 
		{
			std::cerr << "ERROR DELETE" << std::endl;
			return (404);
		}
	}
	else
	{
		std::cerr << "ERROR METHOD" << std::endl;
		return (405);
	}
	return (404);
}

/**
 * @brief	Processes the response for the request.
 * @details	This method handles the response based on whether the requested path is a directory or a file.
 * 			It initializes CGI if applicable and reads the file content if not.
 * @return	Returns 200 if the response is processed successfully, or an error code if there is an issue.
 */
int Request::responseProcess()
{
	// list the directory
	if (this->_isDirectory)
	{
		this->_fileExt = ".html";
		//std::cerr << "Directory requested : " << this->_finalPath << std::endl;
		if (this->_route->getAutoIndex() == 0)
		{
			std::cerr << "ERROR Directory requested : " << this->_finalPath << std::endl;
			return (403);
		}
		return(this->listDir(this->_route)); 
	}
	// init cgi and add it to epoll
	this->_fileExt = getExtension(this->_finalPath);
	if (this->_route->getCgiAccept() && isCGI(this->_finalPath))
	{			
		initEnv();
		if (this->_cgi == NULL)
		{
			this->_cgi = new CGI(CGI_PATH, this->_env, this->_server->getEpollFd(), \
				this->_body);
		}
		if (this->_cgi && !this->_cgi->cgiStart())
		{
			std::cerr << "ERROR CGI" << std::endl;
			int error = this->_cgi->getError();
			if (this->_cgi != NULL)
			{
				delete this->_cgi;
				this->_cgi = NULL;
			}
			return (error);
		}
		//this->_cgi->cgiDisplayLog();
		std::cerr << "[CGI] is started" << std::endl;
		return (-1);
	}
	else 
		return(readFile(this->_finalPath));
}



/**
 * @brief	Checks the request and processes it based on the event and timeout.
 * @details	This method checks if a route exists, handles CGI requests, and processes the request based on the event type.
 * 			It returns an appropriate HTTP status code based on the request processing result.
 * @param	event	A pointer to an epoll_event structure for handling events.
 * @param	timeout	The timeout value for epoll_wait.
 * @return	Returns 200 if the request is processed successfully, or an error code if there is an issue.
 */
int	Request::checkRequest(struct epoll_event *event, bool timeout)
{
	if (!this->_route)
		return (404);
	if (!event) // if event is NULL, we are not in a cgi request
	{
		if(this->_cgi && timeout == true)
		{
			std::cerr << "[CGI] Timeout reached, handling CGI timeout" << std::endl;
			return (this->handleCGITimeout());
		}
		if (this->_cgi == NULL && timeout == false)
		{
			return (responseProcess());
		}
		// If timeout is not reached, we are still waiting for the CGI response
	}
	else
	{
		if (this->_cgi)
		{
			return (this->handleCGIEvent(event, timeout));
		}
	}

	std::cerr << "[CGI] Not finished" << std::endl;
	return (-1);
}

/**
 * @brief Handles the CGI process communication via epoll events.
 * 
 * @param event     Pointer to the current epoll event.
 * @param timeout   Timeout value passed from the event loop.
 * @return          HTTP status code returned by CGI handlers or -1 if not handled.
 */
int Request::handleCGIEvent(struct epoll_event *event, int timeout)
{

	//std::cerr << "[CGI] is running" << std::endl;

	// Handle CGI write to stdin
	if ((event->events & EPOLLOUT) && event->data.fd == this->_cgi->getStdinPipe())
	{
		std::cerr << "[CGI] is writing" << std::endl;
		return (this->_cgi->cgiWrite(event, timeout));
	}

	// Handle CGI read from stdout
	if ((event->events & EPOLLIN) && event->data.fd == this->_cgi->getStdoutPipe())
	{
		std::cerr << "[CGI] is reading" << std::endl;
		int ret = this->_cgi->cgiRead(event, timeout);
		if (ret == 200)
		{
			this->_htmlContent = this->_cgi->getResult();

			delete this->_cgi;
			this->_cgi = NULL;
			std::cerr << "[CGI] finished reading" << std::endl;
			return (200);
		}
		return (ret);
	}
	return (-1);
}
/**
 * @brief Terminates the CGI process and cleans up resources after a timeout.
 * 
 * @return HTTP status code 504 (Gateway Timeout).
 */
int Request::handleCGITimeout()
{
	if (!this->_cgi)
		return (504);

	int epollFd = this->_server->getEpollFd();

	std::cerr << "DELETING CGI pipes " << this->_cgi->getStdoutPipe() << std::endl;
	if (epoll_ctl(epollFd, EPOLL_CTL_DEL, this->_cgi->getStdoutPipe(), NULL) == -1)
		std::cerr << "Failed to remove CGI stdout pipe from epoll: " << strerror(errno) << std::endl;
	std::cerr << "closing CGI pipes " << this->_cgi->getStdoutPipe() << std::endl; 
	if (close(this->_cgi->getStdoutPipe()) == -1)
		std::cerr << "Failed to close CGI stdout pipe: " << strerror(errno) << std::endl;
		
	if (kill(this->_cgi->getPid(), SIGKILL) == -1)
		std::cerr << "Failed to kill CGI process: " << strerror(errno) << std::endl;

	delete this->_cgi;
	this->_cgi = NULL;

	std::cerr << "CGI timeout, deleting CGI" << std::endl;
	return (504);
}




int Request::readFile(std::string &path){
	if (path.empty())
		return (404);
	std::ifstream file(path.c_str());
	if (file.is_open())
	{
		std::ostringstream contentStream;
		contentStream << file.rdbuf();
		this->_htmlContent =  contentStream.str();
		file.close();
		return (200);
	}
	else
	{
		std::cerr << "ERROR open file from request" << std::endl;
		return (404);
	}
}

bool	Request::checkReturn(Route *route){
	if (route->getRedir().exist){
		if (!routeExists(this->_client->getHistoric(), this->_route->getPath()))
			this->_client->getHistoric().push_back(this->_route->getPath());
		this->_headerInfo.url = route->getRedir().path;
		return (true);
	}
	return (false);
}


//********GETTERS
std::map<std::string, std::string> Request::getEnv(void) const
{
	return (this->_env);
}

const t_header &Request::getHeaderInfo() const{
	return (this->_headerInfo);
}

const std::string &Request::getFileExt() const{
	return (this->_fileExt);
}


const std::map<std::string, std::string> &Request::getTypeMime() const{
	return (this->_typeMime);
}