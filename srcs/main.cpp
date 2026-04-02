/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 12:51:15 by glions            #+#    #+#             */
/*   Updated: 2025/05/30 09:06:22 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>
# include <csignal>
# include "Webserv.hpp"

// Default configuration file path
std::string file = "./configs/8000.conf";
Webserv *webserv = NULL;


/**
 * @brief	Signal handler for graceful shutdown.
 *			Handles SIGINT signal to clean up resources and exit the program.
 * @param	signum	The signal number.
 * @details	This function deletes the Webserv instance and clears the configuration file path.
 * 			It is registered to handle the SIGINT signal (Ctrl+C).
 * 			This allows the server to shut down gracefully when interrupted.
 */
void signalHandler(int signum)
{
	file.clear();
	if (webserv)
		delete webserv;
	exit(signum);
}


/**
 * @brief	Entry point of the program.
 *			Handles command-line arguments, sets up signal handling,
 *			and initializes and starts the web server.
 */
int main(int ac, char** av)
{
	
	// Check usage
	if (ac > 2)
	{
		std::cerr << "Usage: ./webserv [config_file]" << std::endl;
		file.clear();
		return (0);
	}

	// If a config file is provided, store it
	else if (ac == 2)
	{
		file.clear();
		file = std::string(av[1]);
	}
	// If no config file is provided, use the default one

	// Set up signal handling for graceful shutdown
	// Register signal handler for SIGINT (Ctrl+C)
	
	if (signal(SIGINT, signalHandler) == SIG_ERR)
	{
		std::cerr << "Error setting up signal handler" << std::endl;
		return (1);
	}

	// Create and initialize Webserv instance
	webserv = new Webserv();
	if (!webserv->parsing(file) ||
		!webserv->initServers() ||
		!webserv->ready() ||
		!webserv->start())
	{
		// Cleanup on failure
		file.clear();
		delete webserv;
		webserv = NULL;
		return (1);
	}

	// Cleanup on success
	file.clear();
	delete webserv;
	webserv = NULL;
	return (0);
}

