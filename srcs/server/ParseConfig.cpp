/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 12:54:05 by glions            #+#    #+#             */
/*   Updated: 2025/05/30 08:47:38 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ParseConfig.hpp"
# include "utils.hpp"

/**
 * @brief	Constructor for the ParseConfig class.
 * @details	This constructor initializes the ParseConfig instance with the specified
 * 			file path and prepares it for parsing.
 * @param	path	The path to the configuration file.
 * @throw	ErrorFileExtension If the file extension is not valid.
 * @throw	ErrorFile If the file cannot be opened.
 */
ParseConfig::ParseConfig(std::string path) : _path(path), _configs()
{
	if (!isValidExtension(this->_path, "conf"))
		throw ParseConfig::ErrorFileExtension();
	this->_file.open(this->_path.c_str());
	if (!this->_file.is_open())
		throw ParseConfig::ErrorFile();
}

/**
 * @brief	Destructor for the ParseConfig class.
 * @details	This destructor cleans up resources used by the ParseConfig instance,
 * 			including closing the file and deleting any ServerConfig objects.
 */
ParseConfig::~ParseConfig(void)
{
	if (this->_file.is_open())
		this->_file.close();
	for (size_t i = 0; i < this->_configs.size(); i++)
	{
		if (this->_configs[i])
			delete this->_configs[i];
	}
	this->_file.clear();
}

/**
 * @brief	Copy operator for the ParseConfig class.
 * @details	This constructor creates a new ParseConfig instance as a copy of another.
 * @param	copy	The ParseConfig instance to copy from.
 */
ParseConfig &ParseConfig::operator=(const ParseConfig &copy)
{
	if (this != &copy)
	{
	}
	return (*this);
}

/**
 * @brief	Starts the parsing process for the configuration file.
 * @details	This method reads the content of the configuration file and processes
 * 			it to extract server configurations. It throws exceptions for various errors.
 * @return	Returns true if parsing is successful, false otherwise.
 * @throw	ErrorFileEmpty If the file is empty.
 * @throw	ErrorFileContent If there is a bad value in the file.
 */
bool ParseConfig::startParsing(void)
{
	std::vector<std::string> contentFile = readFile(this->_file);
	if (contentFile.size() <= 0)
		throw ParseConfig::ErrorFileEmpty();
	for (size_t i = 0; i < contentFile.size(); i++)
	{
		std::vector<std::string> args = splitString(contentFile[i], ' ');
		cleanArgs(&args);
		if (args.size() == 2 && args[0] == "server" && args[1] == "{")
		{
			try
			{
				i++;
				ServerConfig *tmp = this->parseServer(&i, contentFile);
				this->_configs.push_back(tmp);
			}
			catch (std::exception &e)
			{
				std::cerr << e.what() << std::endl;
				return (false);
			}
		}
		else if (args.size() != 0)
		{
			std::cerr << "bad value on file" << std::endl;
			return (false);
		}
	}
	return (true);
}

/**
 * @brief	Signal handler for graceful shutdown.
 *			Handles SIGINT signal to clean up resources and exit the program.
* @param	signum	The signal number.
* @details	This function deletes the Webserv instance and clears the configuration file path.
* 			It is registered to handle the SIGINT signal (Ctrl+C).
* 			This allows the server to shut down gracefully when interrupted.
*/
ServerConfig *ParseConfig::parseServer(size_t *i, std::vector<std::string> lines)
{
	ServerConfig *conf = new ServerConfig();
	while (*i < lines.size())
	{
		std::vector<std::string> args = splitString(lines[*i], ' ');
		cleanArgs(&args);
		// LISTEN
		if (args.size() >= 1 && args[0] == "listen")
		{
			try
			{
				conf->setPort(args);
			} catch (std::exception &e) {
				delete conf;
				std::cerr << "listen : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// SERVER_NAME
		else if (args.size() >= 1 && args[0] == "server_name")
		{
			try
			{
				conf->setServerName(args);
			} catch (std::exception &e) {
				delete conf;
				std::cerr << "server_name : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// ERROR_PAGE
		else if (args.size() >= 1 && args[0] == "error_page")
		{
			try
			{
				conf->addErrorPage(args);
			} catch(const std::exception& e) {
				delete conf;
				std::cerr << "error_page : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// CLIENT_MAX_BODY
		else if (args.size() >= 1 && args[0] == "client_max_body")
		{
			try
			{
				conf->setClientMaxBody(args);
			} catch (std::exception &e) {
				delete conf;
				std::cerr << "client_max_body : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// LOCATION -> NEW ROUTE
		else if (args.size() == 3 && args[0] == "location" && args[2] == "{")
		{
			try
			{
				Route *tmp = this->parseRoute(i, lines);
				conf->addRoute(tmp);
			} catch (std::exception &e) {
				delete conf;
				std::cerr << "location : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// END OF BLOC SERVER
		else if (args.size() == 1 && args[0] == "}")
			return (conf);
		else if (args.size() != 0)
		{
			delete conf;
			throw ParseConfig::ErrorFileContent();
		}
		(*i)++;
	}
	delete conf;
	throw ParseConfig::ErrorFileContent();
}

/**
 * @brief	Parses a route configuration from the provided lines.
 * @details	This method processes the lines to extract route-specific configurations
 * 			such as methods, redirection, root, autoindex, index, and CGI acceptance.
 * @param	i		A pointer to the current index in the lines vector.
 * @param	lines	The vector of lines read from the configuration file.
 * @return	Returns a pointer to a Route object containing the parsed route configuration.
 * @throw	ErrorFileContent If there is a bad value in the file.
 */
Route *ParseConfig::parseRoute(size_t *i, std::vector<std::string> lines)
{
	std::vector<std::string> args = splitString(lines[*i], ' ');
	cleanArgs(&args);
	Route *route = new Route(args[1]);
	while (++(*i) < lines.size())
	{
		args = splitString(lines[*i], ' ');
		cleanArgs(&args);
		// METHODS
		if (args.size() >= 1 && args[0] == "methods")
		{
			try
			{
				route->addMethods(args);
			} catch (std::exception &e) {
				delete route;
				route = NULL;
				std::cerr << "methods : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// RETURN -> REDIRECTION
		else if (args.size() >= 1 && args[0] == "return")
		{
			try
			{
				route->setRedir(args);
			} catch (std::exception &e) {
				delete route;
				route = NULL;
				std::cerr << "return : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// ROOT
		else if (args.size() >= 1 && args[0] == "root")
		{
			try
			{
				route->setRoot(args);
			} catch (std::exception &e) {
				delete route;
				route = NULL;
				std::cerr << "root : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// AUTOINDEX
		else if (args.size() >= 1 && args[0] == "autoindex")
		{
			try
			{
				route->setAutoIndex(args);
			} catch (std::exception &e) {
				delete route;
				route = NULL;
				std::cerr << "autoindex : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		// INDEX
		else if (args.size() >= 1 && args[0] == "index")
		{
			try
			{
				route->setIndex(args);
			} catch (std::exception &e) {
				delete route;
				route = NULL;
				std::cerr << "index : " << e.what() << std::endl;
				throw ParseConfig::ErrorFileContent();
			}
		}
		else if (args.size() >= 2 && args[0] == "cgi")
		{
			try
			{
				route->setCgiAccept(args);
			} catch (std::exception &e)
			{
				delete route;
				route = NULL;
				std::cerr << "cgi : " << e.what() << std::endl;
				throw ParseConfig::ErrorCgi();
			}
		}
		// END OF BLOC ROUTE
		else if (args.size() == 1 && args[0] == "}")
		{
			if (route->getAutoIndex() == 2)
				route->setAutoIndex(0);
			return (route);
		}
		else if (args.size() != 0)
		{
			delete route;
			std::cerr << "format : " << std::endl;
			throw ParseConfig::ErrorFileContent();
		}
	}
	delete route;
	throw ParseConfig::ErrorFileContent();
}


// GETTERS
/**
 * @brief	Retrieves the list of server configurations.
 * @details	This method returns a vector containing pointers to ServerConfig objects
 * 			parsed from the configuration file.
 * @return	A vector of pointers to ServerConfig objects.
 */
std::vector<ServerConfig *> ParseConfig::getConfigs(void) const
{
	return (this->_configs);
}

/**
 * @brief	Retrieves the path of the configuration file.
 * @details	This method returns the path to the configuration file that was set during
 * 			the construction of the ParseConfig instance.
 * @return	A string containing the path to the configuration file.
 */
std::string ParseConfig::getPath(void) const
{
	return (this->_path);
}


// EXCEPTIONS
const char *ParseConfig::ErrorFileExtension::what() const throw()
{
	return ("File extension not valid, must be .conf");
}

const char *ParseConfig::ErrorFile::what() const throw()
{
	return ("File not found");
}

const char *ParseConfig::ErrorFileContent::what() const throw()
{
	return ("Bad value on file");
}

const char *ParseConfig::ErrorFileEmpty::what() const throw()
{
	return ("File is empty");
}

const char *ParseConfig::ErrorCgi::what() const throw()
{
	return ("Argument not valid");
}
