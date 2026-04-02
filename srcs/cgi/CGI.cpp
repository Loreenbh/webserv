/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 17:58:03 by tissad            #+#    #+#             */
/*   Updated: 2025/06/02 12:52:33 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>

# include "CGI.hpp"
# include "CGIUtils.hpp"


/**
 * @brief	Default constructor for the CGI class.
 * @details	This constructor initializes the CGI object with default values.
 * 			It sets up the environment map, path, epoll file descriptor, body,
 * 			environment variables, arguments, error code, and result string.
 * @note	This constructor is used to create a CGI object without any specific parameters.
 * @throw	None
 */
CGI::CGI(void) : \
		_envMap(), _path(""), _epollFd(-1),\
		_body(""), _env(NULL), _args(NULL), _error(0), _result("")
{
	//  std::cerr << "CGI default constructor called" << std::endl;
}	

/**
 * @brief	Constructor for the CGI class with parameters.
 * @details	This constructor initializes the CGI object with a specified path,
 * 			environment variables, and an epoll file descriptor.
 * 			It sets up the body, environment variables, arguments, error code,
 * 			result string, and CGI timer.
 * @param	path	The path to the CGI script.
 * @param	env	A map of environment variables for the CGI script.
 * @param	epollFd	The epoll file descriptor for monitoring events.
 * @note	This constructor is used to create a CGI object with specific parameters.
 * @throw	None
 */

CGI::CGI(std::string path, std::map<std::string, std::string> env, int epollFd) : \
		_envMap(env), _path(path), _epollFd(epollFd), _body(""),  _env(NULL), _args(NULL), \
		_error(0), _result(""), _cgi_timer(std::time(NULL))
{

}

/**
 * @brief	Constructor for the CGI class with parameters and body.
 * @details	This constructor initializes the CGI object with a specified path,
 * 			environment variables, an epoll file descriptor, and a request body.
 * 			It sets up the environment variables, arguments, error code, result string,
 * 			and CGI timer.
 * @param	path	The path to the CGI script.
 * @param	env	A map of environment variables for the CGI script.
 * @param	epollFd	The epoll file descriptor for monitoring events.
 * @param	body	The request body to be processed by the CGI script.
 * @note	This constructor is used to create a CGI object with specific parameters and a request body.
 * @throw	None
 */

CGI::CGI(std::string path, std::map<std::string, std::string> env, int epollFd, std::string body) :\
		_envMap(env), _path(path), _epollFd(epollFd), _body(body),  _env(NULL), _args(NULL), \
		_error(0), _result(""), _cgi_timer(std::time(NULL))
{
	//  std::cerr << "CGI constructor called" << std::endl;
}

CGI::~CGI()
{
	if (this->_env)
	{
		for (size_t i = 0; this->_env[i]; ++i)
		{
			if (this->_env[i])
			{
				delete[] (this->_env[i]);
				this->_env[i] = NULL;
			}
		}
		delete[] (this->_env);
		this->_env = NULL;
	}
	if (this->_args)
	{
		for (size_t i = 0; this->_args[i]; ++i)
		{
			if(this->_args[i])
			{
				delete[] (this->_args[i]);
				this->_args[i] = NULL;
			}
		}
		delete[] (this->_args);
		this->_args = NULL;
	}
	if (kill(this->_pid, 0) != -1)
		kill(this->_pid, SIGTERM);
	if (this->_stdin_pipe[0] != -1)
	{
		close(this->_stdin_pipe[0]);
		this->_stdin_pipe[0] = -1;
	}
	if (this->_stdin_pipe[1] != -1)
	{
		close(this->_stdin_pipe[1]);
		this->_stdin_pipe[1] = -1;
	}
	if (this->_stdout_pipe[0] != -1)
	{
		close(this->_stdout_pipe[0]);
		this->_stdout_pipe[0] = -1;
	}
	if (this->_stdout_pipe[1] != -1)
	{
		close(this->_stdout_pipe[1]);
		this->_stdout_pipe[1] = -1;
	}
}


/**
 * @brief	Starts the CGI process by building the environment
 * and arguments, creating pipes, and executing the CGI script.
 * @details	This method builds the environment and arguments for the CGI script,
 * 			creates pipes for communication, and executes the CGI script using `execve`.
 * 			It handles errors during the process and returns a boolean indicating success or failure.
 * @param	None
 * @return	Returns true if the CGI process was started successfully, false otherwise.
 * @note	This method is the main entry point for starting the CGI process.
 * @throw	None
 */
bool CGI::cgiBuildEnv(void)
{
	this->_env = new char*[this->_envMap.size() + 1];
	
	int i = 0;
	for (std::map<std::string, std::string>::iterator it = this->_envMap.begin();\
		it != this->_envMap.end(); ++it)
	{
		std::string str = it->first + "=" + it->second;
		try
		{
			this->_env[i] = new char[str.size() + 1];
			std::strcpy(this->_env[i], str.c_str());
			++i;
		}
		catch (std::bad_alloc &e)
		{
			return (this->cgiAddLog(e.what(), __LINE__), \
			this->_error = ERROR_500, false);
		}
	}
	this->_env[i] = NULL;
	return (true);
}


/**
 * @brief	Executes the CGI script using `execve`.
 * @details	This method forks a new process and executes the CGI script using `execve`.
 * 			It redirects the standard input and output to the pipes created earlier.
 * 			If successful, it returns true; otherwise, it logs the error and returns false.
 * @param	None
 * @return	Returns true if the CGI script was executed successfully, false otherwise.
 * @note	This method is called after building the environment and arguments.
 * @throw	None
 */
bool CGI::cgiBuildArgs(std::string str)
{
	std::vector<std::string> ar = splitString(str, SEP);
	try
	{
		this->_args = new char*[ar.size() + 2];
		this->_args[0] = new char[this->_path.size() + 1];
		std::strcpy(this->_args[0], this->_path.c_str());
	}
	catch (std::bad_alloc &e)
	{
		return (this->cgiAddLog(e.what(), __LINE__), \
			this->_error = ERROR_500, false);
	}
	size_t i = 0;
	for ( ; i < ar.size(); i++)
	{
		try
		{
			this->_args[i + 1] = new char[ar[i].size() + 1];
			std::strcpy(this->_args[i + 1], ar[i].c_str());
		}
		catch (std::bad_alloc &e)
		{
			return (this->cgiAddLog(e.what(), __LINE__), \
			this->_error = ERROR_500, false);
		}
	}
	this->_args[i + 1] = NULL;
	return (true);
}


/**
 * @brief	Builds the CGI environment and arguments.
 * @details	This method builds the CGI environment and arguments by calling
 * 			`cgiBuildArgs` and `cgiBuildEnv`. It returns true if both methods succeed,
 * 			otherwise it logs the error and returns false.
 * @param	None
 * @return	Returns true if the CGI environment and arguments were built successfully, false otherwise.
 * @note	This method is called before executing the CGI script.
 * @throw	None
 */
bool CGI::cgiBuild(void)
{
	std::string scriptFileName = this->_envMap["SCRIPT_FILENAME"];
	return (this->cgiBuildArgs(scriptFileName) && \
		this->cgiBuildEnv());
}

/**
 * @brief	Adds a log message to the CGI logs.
 * @details	This method appends a log message to the `_logs` vector with the specified line number.
 * @param	log	The log message to be added.
 * @param	line	The line number where the log was generated.
 * @note	This method is used for logging errors and information during CGI processing.
 * @throw	None
 */
void CGI::cgiAddLog(std::string log, int line)
{
	this->_logs.push_back("CGI: "  + log + " at line " + toString(line));
}

/**
 * @brief	Creates pipes for CGI communication.
 * @details	This method creates pipes for standard input and output communication
 * 			for the CGI process. It returns true if the pipes were created successfully,
 * 			otherwise it logs the error and returns false.
 * @param	None
 * @return	Returns true if the pipes were created successfully, false otherwise.
 * @note	This method is called before executing the CGI script.
 * @throw	None
 */
bool CGI::cgiCreatePipe(void)
{
	if (pipe(this->_stdin_pipe) == -1 || pipe(this->_stdout_pipe) == -1)
	{
		return (this->cgiAddLog(strerror(errno), __LINE__), \
		this->_error = ERROR_500, false);
	}
	std::cout << "CGI pipes created successfully" << std::endl;
	std::cerr << "stdin_pipe[0]: " << this->_stdin_pipe[0] << ", stdin_pipe[1]: " << this->_stdin_pipe[1] << std::endl;
	std::cerr << "stdout_pipe[0]: " << this->_stdout_pipe[0] << ", stdout_pipe[1]: " << this->_stdout_pipe[1] << std::endl;
	return (true);
}


/**
 * @brief	Redirects the child process's standard input and output to the pipes.
 * @details	This method closes the write end of the stdin pipe and the read end of the stdout pipe,
 * 			and redirects the standard input and output to the pipes using `dup2`.
 * 			It returns true if the redirection was successful, otherwise it logs the error and returns false.
 * @param	None
 * @return	Returns true if the redirection was successful, false otherwise.
 * @note	This method is called in the child process after forking.
 * @throw	None
 */
bool CGI::cgiRedirectChild(void)
{
	if (close(this->_stdin_pipe[1]) == -1 || \
		close(this->_stdout_pipe[0]) == -1)
	{
		return (this->cgiAddLog(strerror(errno), __LINE__), \
			this->_error = ERROR_500, false);
	}
	if (dup2(this->_stdin_pipe[0], STDIN_FILENO) == -1 || \
		dup2(this->_stdout_pipe[1], STDOUT_FILENO) == -1)
	{
		return (this->cgiAddLog(strerror(errno), __LINE__), \
			this->_error = ERROR_500, false);
	}
	if (close(this->_stdin_pipe[0]) == -1 || \
		close(this->_stdout_pipe[1]) == -1)
	{
		return (this->cgiAddLog(strerror(errno), __LINE__), \
			this->_error = ERROR_500, false);
	}
	return (true);
}


/**
 * @brief	Reads data from the CGI process's standard output.
 * @details	This method reads data from the CGI process's standard output pipe
 * 			and stores it in the `_result` string. It handles EOF and errors,
 * 			and returns the appropriate HTTP status code based on the read result.
 * @param	event	The epoll event containing the file descriptor to read from.
 * @param	timeout	The timeout value for reading data.
 * @return	Returns 200 if the read was successful, or an error code if there was an issue.
 * @note	This method is called when the CGI process has data available to read.
 * @throw	None
 */
int  CGI::cgiRead(struct epoll_event *event, int timeout)
{
	std::string res;
	char buffer[BUFFER_SIZE];
	ssize_t bytesRead;
	int readen = 0;
	(void)timeout; // to avoid unused variable warning

	this->_cgi_timer += std::time(NULL) - this->_cgi_timer; // increment timer for cgi timeout

	while ((bytesRead = read(event->data.fd, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[bytesRead] = '\0';
		res += buffer;
		readen += bytesRead;
	}

	if (bytesRead == 0) { // EOF
		this->_result = res;
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, event->data.fd, NULL))
		{
			std::cerr << "Error removing CGI stdout pipe from epoll: " << strerror(errno) << std::endl;
			close(event->data.fd);
			return (ERROR_500);
		}
		// std::cout << "fd " << event->data.fd << " closed" << std::endl;
		if (close(event->data.fd) == -1)
		{
			std::cerr << "Error closing CGI stdout pipe: " << strerror(errno) << std::endl;
			this->_stdout_pipe[0] = -1;
			return (ERROR_500);
		} 
		//std::cerr << readen << " bytes read from CGI stdout at line " << __LINE__ << std::endl;
		this->cgiAddLog(toString(readen) + " bytes read from CGI stdout at line ", __LINE__);
		return (200); // return 200 OK
	}

	if (bytesRead == -1) {
		this->cgiAddLog(strerror(errno), __LINE__);
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, event->data.fd, NULL) == -1)
		{
			std::cerr << "Error removing CGI stdout pipe from epoll: " << strerror(errno) << std::endl;
			close(event->data.fd);
			return (ERROR_500);
		}
		if (close(event->data.fd) == -1)
		{
			this->_stdout_pipe[0] = -1;
			std::cerr << "Error closing CGI stdout pipe: " << strerror(errno) << std::endl;
		}
		this->_stdout_pipe[0] = -1;
		return (ERROR_500);
	}

	this->cgiAddLog("Erreur : CGI process not finished", __LINE__);
	return (ERROR_500); // return 500 Internal Server Error
}

/**
 * @brief	Writes data to the CGI process's standard input.
 * @details	This method writes the request body to the CGI process's standard input pipe.
 * 			It handles errors during writing and closes the pipe after writing is complete.
 * @param	event	The epoll event containing the file descriptor to write to.
 * @param	timeout	The timeout value for writing data.
 * @return	Returns -1 if the write was successful, or an error code if there was an issue.
 * @note	This method is called when the CGI process is ready to receive data.
 * @throw	None
 */
int CGI::cgiWrite(struct epoll_event *event, int timeout)
{
	int written = 0;
	ssize_t n;
	(void)timeout;

	while (written < (int)this->_body.size())
	{
		n = write(event->data.fd, this->_body.c_str() + written, this->_body.size() - written);
		if (n < 0) {
			epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, event->data.fd, NULL);
			close(event->data.fd);
			std::cerr << "Error writing to CGI stdin: " << strerror(errno) << std::endl;
			return (ERROR_500);
		}
		written += n;
	}
	
	epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, event->data.fd, NULL);
	close(event->data.fd);
	this->cgiAddLog("Wrote " + toString(written) + " bytes to CGI stdin at line ", __LINE__);
	return (-1); //ok
}


/**
 * @brief	Adds a file descriptor to the epoll instance for monitoring events.
 * @details	This method creates an `epoll_event` structure, sets the events and file descriptor,
 * 			and adds it to the epoll instance using `epoll_ctl`. It returns true if successful,
 * 			otherwise it logs the error and returns false.
 * @param	fd	The file descriptor to be added to the epoll instance.
 * @param	event	The events to monitor for the file descriptor (e.g., EPOLLIN, EPOLLOUT).
 * @return	Returns true if the file descriptor was added successfully, false otherwise.
 * @note	This method is used to monitor file descriptors for events in the CGI process.
 * @throw	None
 */

bool CGI::epollAddFd(int fd, int event)
{
	struct epoll_event ev = {};
	ev.events = event;
	ev.data.fd = fd;
	// add the file descriptor to the epoll instance
	if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		return (this->cgiAddLog(std::string(strerror(errno)), __LINE__), \
			this->_error = ERROR_500, false);
	}
	// add log
	this->cgiAddLog("epollAddFd: fd " + toString(fd) + " added", __LINE__);
	return (true);
}

/**
 * @brief	Executes the CGI script by forking a new process and redirecting input/output.
 * @details	This method creates pipes for standard input and output, forks a new process,
 * 			redirects the standard input and output to the pipes, and executes the CGI script using `execve`.
 * 			It handles errors during the process and returns a boolean indicating success or failure.
 * @param	None
 * @return	Returns true if the CGI script was executed successfully, false otherwise.
 * @note	This method is called after building the CGI environment and arguments.
 * @throw	None
 */

bool	CGI::cgiExec(void)
{
	// create pipes for stdin, stdout and stderr	
	if (!this->cgiCreatePipe())
		return (this->cgiAddLog(std::string(strerror(errno)), __LINE__), \
			this->_error = ERROR_500, false);
	// fork a new process
	this->_pid = fork();
	if (this->_pid == -1)
		return (this->cgiAddLog(std::string(strerror(errno)), __LINE__), \
			this->_error = ERROR_500, false);
	if (this->_pid == 0){
		// child process
		// redirect stdin, stdout and stderr to the pipes
		if (!this->cgiRedirectChild())
			_exit(ERROR_500);
		execve(this->_args[0], this->_args, this->_env);
		// if execve fails, exit with error
		exit(ERROR_500);
	}else{
		// parent process
		// close the write end of the stdin pipe
		if (close(this->_stdin_pipe[0]) == -1 || \
			close(this->_stdout_pipe[1]) == -1 )
		{
			return (this->cgiAddLog(std::string(strerror(errno)), __LINE__), \
				this->_error = ERROR_500, false);
		}
		this->_stdin_pipe[0] = -1;
		this->_stdout_pipe[1] = -1;
		// add the file descriptor to the epoll instance
		if (!epollAddFd(this->_stdin_pipe[1], EPOLLOUT)
		  || !epollAddFd(this->_stdout_pipe[0], EPOLLIN))
		{
			return (this->cgiAddLog(std::string(strerror(errno)), __LINE__), \
				false);
		}	
		return (true);
	}
}

/**
 * @brief	Cleans up the memory used by the CGI object.
 * @details	This method deletes the environment variables and arguments arrays,
 * 			freeing the memory allocated for them. It sets the pointers to NULL after deletion.
 * @param	None
 * @return	None
 * @note	This method is called to clean up resources when the CGI object is no longer needed.
 * @throw	None
 */

void CGI::memoryCleanUp(void)
{
	if (this->_env)
	{
		for (size_t i = 0; this->_env[i]; ++i)
		{
			if (this->_env[i])
			{
				delete[] (this->_env[i]);
				this->_env[i] = NULL;
			}
		}
		delete[] (this->_env);
		this->_env = NULL;
	}
	if (this->_args)
	{
		for (size_t i = 0; this->_args[i]; ++i)
		{
			if(this->_args[i])
			{
				delete[] (this->_args[i]);
				this->_args[i] = NULL;
			}
		}
		delete[] (this->_args);
		this->_args = NULL;
	}
}


/**
 * @brief	Starts the CGI process by building the environment and executing the script.
 * @details	This method builds the CGI environment and arguments, executes the CGI script,
 * 			and handles the result. It cleans up resources after execution and returns a boolean
 * 			indicating success or failure.
 * @param	None
 * @return	Returns true if the CGI process was started successfully, false otherwise.
 * @note	This method is the main entry point for starting the CGI process.
 * @throw	None
 */

bool CGI::cgiStart(void)
{
	if (this->cgiBuild())
	{
		this->cgiAddLog("CGI build success", __LINE__);
	}
	else
	{
		this->cgiAddLog("CGI build failed", __LINE__);
		this->cgiDisplayLog();
		this->memoryCleanUp();
		return (false);
	}
	if (this->cgiExec())
	{
		this->cgiAddLog("CGI exec success", __LINE__);
		this->memoryCleanUp();
	}
	else
	{
		this->cgiAddLog("CGI exec failed", __LINE__);
		this->cgiDisplayLog();
		this->memoryCleanUp();
		return (false);
	}
	//this->cgiDisplayLog();
	return (true);
}

/**
 * @brief	Gets the result of the CGI execution.
 * @details	This method returns the result string containing the output of the CGI script.
 * @param	None
 * @return	Returns the result string of the CGI execution.
 * @note	This method is used to retrieve the output after the CGI script has been executed.
 * @throw	None
 */

std::string CGI::getResult(void) const
{
	return this->_result;
}

/**
 * @brief	Displays the CGI logs, environment variables, and arguments for debugging.
 * @details	This method prints the CGI logs, environment variables, and arguments to the standard error output.
 * 			It is useful for debugging purposes to see the internal state of the CGI object.
 * @param	None
 * @return	None
 * @note	This method is used for debugging and logging the CGI process details.
 * @throw	None
 */

void CGI::cgiDisplayLog(void)
{
	
	
	//print env
	std::cerr << "************************************ENV*****************************" << std::endl;
	std::cerr << "Path CGI: " << this->_path << std::endl;
	for (size_t i = 0; this->_env[i]; ++i)
	{
		std::cerr <<  this->_env[i] << std::endl;
	}
	//print args
	std::cerr << "*********************************ARG*********************************" << std::endl;
	for (size_t i = 0; this->_args[i]; ++i)
	{
		std::cerr  << this->_args[i] << std::endl;
	}
	//print logs
	std::cerr << "********************************LOG*********************************" << std::endl;
	for (size_t i = 0; i < this->_logs.size(); ++i)
	{
		std::cerr << this->_logs[i] << std::endl;
	}
	std::cerr << "********************************************************************" << std::endl;
}