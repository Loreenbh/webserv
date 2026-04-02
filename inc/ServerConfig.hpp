/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glions <glions@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:29:40 by glions            #+#    #+#             */
/*   Updated: 2025/04/14 13:50:59 by glions           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "utils.hpp"
# include "Route.hpp"

class ServerConfig
{
    public:
        ServerConfig(void);
        ServerConfig(ServerConfig const &copy);
        ~ServerConfig();
        ServerConfig &operator=(const ServerConfig &copy);
        void print(void);
        // SETTERS + ADD
        void setServerName(std::string name);
        void setServerName(std::vector<std::string> args);
        void setPort(std::vector<std::string> args);
        void setPort(int port);
        void addRoute(Route *route);
        void addErrorPage(std::vector<std::string> args);
        void setClientMaxBody(std::vector<std::string> args);
        // GETTERS
        int getPort(void) const;
        size_t getClientMaxBody(void) const;
        std::string getServerName(void) const;
        std::map<std::string, Route *> getRoutes(void) const;
        std::map<int, std::string> getErrorPages(void) const;
    private:
        std::string _serverName;
        int _port;
        size_t _clientMaxBody;
        std::map<int, std::string> _errorPages;
        std::map<std::string, Route *> _routes;
    class ErrorToManyArgs : public std::exception
    {
        public:
            virtual const char *what() const throw();
    };
    class ErrorNotEnoughArgs : public std::exception
    {
        public:
            virtual const char *what() const throw();
    };
    class ErrorNotValidArgs : public std::exception
    {
        public:
            virtual const char *what() const throw();
    };
    class ErrorErrorPageAlreadyIn : public std::exception
    {
        public:
            virtual const char *what() const throw();
    };
    class ErrorDuplicate : public std::exception
    {
        public:
            virtual const char *what() const throw();
    };
};

#endif