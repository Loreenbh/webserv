/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glions <glions@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 09:21:24 by glions            #+#    #+#             */
/*   Updated: 2025/04/14 14:24:03 by glions           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Route.hpp"

Route::Route(void):
    _path(),
    _root(),
    _index(),
    _bodysize(),
    _autoindex(2),
    _methods(),
    _cgiAccept(false),
    _redir()
{
    this->_redir.exist = false;
}

Route::Route(std::string path):
    _path(path),
    _root(),
    _index(),
    _bodysize(),
    _autoindex(2),
    _methods(),
    _cgiAccept(false),
    _redir()
{
    this->_redir.exist = false;
}

Route::Route(const Route &copy):
    _path(copy.getPath()),
    _root(copy.getRoot()),
    _index(copy.getIndex()),
    _bodysize(copy.getBodySize()),
    _autoindex(copy.getAutoIndex()),
    _cgiAccept(copy.getCgiAccept()),
    _redir(copy.getRedir())
{
    std::vector<Method> methods = copy.getMethods();
    for (size_t i = 0; i < methods.size(); i++)
        this->_methods.push_back(methods[i]);
}

Route::~Route(){}

void Route::print(void)
{   
    std::cout << "Route " << this->_path << std::endl;
    std::cout << "Root : " << this->_root << std::endl;
    std::cout << "Index : " << this->_index << std::endl;
    std::cout << "Body size : " << this->_bodysize << std::endl;
    std::cout << "Autoindex : " << this->_autoindex << std::endl;
    std::cout << "CGI :" << this->_cgiAccept << std::endl;
    std::cout << "Methods : ";
    for (size_t i = 0; i < this->_methods.size(); i++)
        std::cout << this->_methods[i] << " ";
    std::cout << std::endl;
    std::cout << "Redirection : ";
    if (!this->_redir.exist)
        std::cout << "empty";
    else
        std::cout << "code=" << this->_redir.code << " path=" << this->_redir.path << std::endl;
    std::cout << std::endl;
}

// SETTERS
void Route::setAutoIndex(int value)
{
    this->_autoindex = value;
}

void Route::setAutoIndex(std::vector<std::string> args)
{
    if (this->_autoindex != 2)
        throw Route::ErrorDuplicate();
    if (args.size() > 2)
        throw Route::ErrorToManyArgs();
    if (args.size() < 2)
        throw Route::ErrorNotEnoughArgs();
    if (args[1] == "on" || args[1] == "ON")
        this->_autoindex = 1;
    else if (args[1] == "off" || args[1] == "OFF")
        this->_autoindex = 0;
    else
        throw Route::ErrorNotValidArgs();
}

void Route::setCgiAccept(std::vector<std::string> args)
{
    if (args.size() > 2)
        throw Route::ErrorToManyArgs();
    if (args.size() < 2)
        throw Route::ErrorNotEnoughArgs();
    if (args[1] == "on" || args[1] == "ON")
        this->_cgiAccept = true;
    else if (args[1] == "off" || args[1] == "OFF")
        this->_cgiAccept = false;
    else
        throw Route::ErrorNotValidArgs();
}

void Route::setBodySize(int size)
{
    this->_bodysize = size;
}

void Route::setPath(std::string path)
{
    this->_path = path;
}

void Route::setIndex(std::string index)
{
    this->_index = index;
}

void Route::setIndex(std::vector<std::string> args)
{
    if (this->_index != "")
        throw Route::ErrorDuplicate();
    if (args.size() > 2)
        throw Route::ErrorToManyArgs();
    if (args.size() < 2)
        throw Route::ErrorNotEnoughArgs();
    this->_index = args[1];
}

void Route::setRoot(std::string root)
{
    this->_root = root;
}

void Route::setRoot(std::vector<std::string> args)
{
    if (this->_root != "")
        throw Route::ErrorDuplicate();
    if (args.size() > 2)
        throw Route::ErrorToManyArgs();
    if (args.size() < 2)
        throw Route::ErrorNotEnoughArgs();
    this->_root = args[1];
}

void Route::addMethod(Method m)
{
    this->_methods.push_back(m);
}

bool alreadyIn(Method m, std::vector<Method> tab)
{
    for (size_t i = 0; i < tab.size(); i++)
    {
        if (tab[i] == m)
            return (true);
    }
    return (false);
}

void Route::addMethods(std::vector<std::string> args)
{
    if (this->_methods.size() != 0)
        throw Route::ErrorDuplicate();
    if (args.size() < 2)
        throw Route::ErrorNotEnoughArgs();
    for (size_t i = 1; i < args.size(); i++)
    {
        if (args[i] == "GET" && !alreadyIn(GET, this->_methods))
            this->_methods.push_back(GET);
        else if (args[i] == "POST" && !alreadyIn(POST, this->_methods))
            this->_methods.push_back(POST);
        else if (args[i] == "DELETE" && !alreadyIn(DELETE, this->_methods))
            this->_methods.push_back(DELETE);
        else
            throw Route::ErrorNotValidArgs();
    }
}

void Route::setRedir(t_redirection redir)
{
    this->_redir = redir;
}

void Route::setRedir(std::vector<std::string> args)
{
    if (this->_redir.exist == true)
        throw Route::ErrorDuplicate();
    if (args.size() > 3)
        throw Route::ErrorToManyArgs();
    if (args.size() < 3)
        throw Route::ErrorNotEnoughArgs();
    for (size_t i = 0; i < args[1].size(); i++)
    {
        if (!isdigit(args[1].at(i)))
            throw Route::ErrorNotValidArgs();
    }
    std::istringstream iss(args[1]);
    int num;
    if (!(iss >> num))
        throw Route::ErrorNotValidArgs();
    this->_redir.exist = true;
    this->_redir.code = num;
    this->_redir.path = args[2];
}

// GETTERS

bool Route::getCgiAccept(void) const
{
    return (this->_cgiAccept);
}

std::string Route::getPath(void) const
{
    return (this->_path);
}

std::string Route::getRoot(void) const
{
    return (this->_root);
}

std::string Route::getIndex(void) const
{
    return (this->_index);
}

int Route::getBodySize(void) const
{
    return (this->_bodysize);
}

int Route::getAutoIndex(void) const
{
    return (this->_autoindex);
}

std::vector<Method> Route::getMethods(void) const
{
    return (this->_methods);
}

t_redirection Route::getRedir(void) const
{
    return (this->_redir);
}


// EXCEPTIONS
const char *Route::ErrorNotEnoughArgs::what() const throw()
{
    return ("not enough args");
}

const char *Route::ErrorToManyArgs::what() const throw()
{
    return ("to much args");
}

const char *Route::ErrorNotValidArgs::what() const throw()
{
    return ("not valid args");
}

const char *Route::ErrorDuplicate::what() const throw()
{
    return ("duplicate rules on file");
}
