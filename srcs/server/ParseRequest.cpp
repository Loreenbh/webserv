/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseRequest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glions <glions@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 11:10:36 by glions            #+#    #+#             */
/*   Updated: 2025/06/02 09:31:08 by glions           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseRequest.hpp"

ParseRequest::ParseRequest(std::string buffer,
    ssize_t bytes, Server *serv, Client *client) :
    _header(),
    _body(),
    _buffer(buffer),
    _bytesRead(bytes),
    _copyRequest(),
    _saveRequest(),
    _isDirectory(false),
    _startBloc(true),
    _hasContentLength(false),
    _sizeBloc(-1),
    _readBloc(0),
    _route(NULL),
    _server(serv),
    _client(client)
{
    this->_headerInfo.accept = "";
    this->_headerInfo.acceptEncoding = "";
    this->_headerInfo.acceptLanguage = "";
    this->_headerInfo.agent = "";
    this->_headerInfo.boundary = "";
    this->_headerInfo.connection = "";
    this->_headerInfo.contentLength = -1;
    this->_headerInfo.contentType = "";
    this->_headerInfo.hostName = "";
    this->_headerInfo.transferEncoding = false;
    this->_headerInfo.url = "";
    this->_headerInfo.version = "";
}

ParseRequest::~ParseRequest()
{
    
}

void ParseRequest::addBuffer(std::string buffer, ssize_t bytes)
{
    this->_buffer.append(buffer, 0, bytes);
}

void ParseRequest::addBody(std::string buffer, ssize_t bytes)
{
    this->_body.append(buffer, 0, bytes);
    this->_bytesRead += bytes;
}

/* *********** PARSING *********** */

void ParseRequest::parseBody(void)
{
    this->_headerInfo.contentLength = 0;
    bool run = true;
    size_t cursor = 0;
    while (run)
    {
        int size;
        size_t pos = this->_body.find("\r\n");
        if (pos != std::string::npos)
        {
            size = hexToInt(this->_body.substr(cursor));
            this->_headerInfo.contentLength += size;
            this->_body.erase(cursor, pos + 2);
            if (size == 0)
                run = false;
            else
                cursor += size + 2;
        }
    }

}

int ParseRequest::parseRequest(std::string request)
{
    std::stringstream ss(request);
    std::string token;
    
    while (ss >> token){
        if (token == "GET")
            this->_headerInfo.method = GET;
        else if (token == "POST")
            this->_headerInfo.method = POST;
        else if (token == "DELETE")
            this->_headerInfo.method = DELETE;
        else if (token[0] == '/')
            this->_headerInfo.url = token;
        else if (token.find("HTTP/") != std::string::npos)
            this->_headerInfo.version = token;
        else
        {
            std::cout << "ERROR 1" << std::endl;
            return (400);
        }
    }
    if (this->_headerInfo.url.size() > 8192)
        return (414);
    this->_headerInfo.uri = this->_headerInfo.url;
    this->cutQueryString();
    this->cutPathInfo();
    if (this->_headerInfo.url.size() == 0)
    {
        std::cout << "ERROR 400 : url empty" << std::endl;
        return (400);
    }
    //std::cerr << "query string : " << this->_headerInfo.queryString << std::endl;
    this->_route = this->findLocation();
    //std::cerr << "URI : " << this->_headerInfo.uri << std::endl;

    if (this->_route == NULL)
        return (404); // tahar : renvoyer un 404 si la route n'existe pas
    std::vector<Method> methods = this->_route->getMethods();
    std::vector<Method>::iterator it = std::find(methods.begin(), methods.end(),
        this->_headerInfo.method);
    if (it == methods.end())
        return (405); // tahar : renvoyer un 405 si la methode n'est pas autorisee au client 
    if (this->_headerInfo.version != SERVER_PROTOCOL)
        return (505); // tahar : renvoyer un 505 si la version n'est pas supportee
    //std::cerr << "========>URL : " << this->_headerInfo.url << std::endl;
    return (this->initFinalPath(this->_route));
}

int ParseRequest::parseHost(std::string host)
{
    std::vector<std::string> args = splitString(host, ' ');
    if (args.size() != 2)
    {
        std::cout << "ERROR 2" << std::endl;
        return (400);
    }
    args = splitString(args[1], ':');
    if (args[0] != this->_server->getConfig()->getServerName())
    {
        std::cout << "ERROR 3" << std::endl;
        return (400);
    }
    this->_headerInfo.hostName = args[0];
    return (0);
}

int ParseRequest::parseHeader(void)
{
    std::string line;
    std::istringstream stream(this->_header);
    int error;
    if (std::getline(stream, line) && (error = parseRequest(line)) != 0)
        return (error);
    while (std::getline(stream, line))
    {
        if (!line.compare(0, 5, "Host:") && (error = parseHost(line)) != 0)
            return (error);
        else if (!line.compare(0, 7, "Accept:"))
            this->_headerInfo.accept = line.substr(8);
        else if (!line.compare(0, 11, "User-Agent:"))
            this->_headerInfo.agent = line.substr(12);
        else if (!line.compare(0, 11, "Connection:"))
            this->_headerInfo.connection = line.substr(12);
        else if (!line.compare(0, 16, "Accept-Language:"))
            this->_headerInfo.acceptLanguage = line.substr(17); 
        else if (!line.compare(0, 16, "Accept-Encoding:"))
            this->_headerInfo.acceptEncoding = line.substr(17);
        else if (!line.compare(0, 13, "Content-Type:"))
            this->_headerInfo.contentType = line.substr(14, line.size() - 15);
    }
     // tester si content length et transfer encoding sont present
    if (this->_headerInfo.transferEncoding && this->_hasContentLength)
    {
        std::cout << "ERROR 4" << std::endl;
        return (400);
    }
    if (this->_headerInfo.contentLength != -1 && 
            this->_server->getConfig()->getClientMaxBody() < (size_t)this->_headerInfo.contentLength)
        return (413);
    return (0);
}

void ParseRequest::cutQueryString(void)
{
    size_t posQuery = this->_headerInfo.url.find("?");
    if (posQuery != std::string::npos)
    {
        this->_headerInfo.queryString = this->_headerInfo.url.substr(posQuery + 1);
        this->_headerInfo.url.erase(posQuery);
    }

}

void ParseRequest::cutPathInfo(void)
{
    size_t pos = this->_headerInfo.url.find(".");
    if (pos != std::string::npos)
    {
        std::string tmp = this->_headerInfo.url.substr(pos+1);
        pos = tmp.find("/");
        if (pos != std::string::npos)
        {
            // std::cerr << "tmp : " << tmp << std::endl;
            this->_headerInfo.pathInfo = tmp.substr(pos);
            //std::cerr << "==>PATH INFO : " << this->_headerInfo.pathInfo << std::endl;
            pos = this->_headerInfo.url.find(this->_headerInfo.pathInfo);
            if (pos != std::string::npos)
            {
                this->_headerInfo.url.erase(pos);
                this->_headerInfo.pathInfo.erase(0, 1);
            }
        }
    }
}

std::vector<std::string> doSplit(const std::string& str, char delimiter){
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    if (str[0] == '/'){
        result.push_back("/");
        start = 1;
        end = str.find(delimiter, start); 
    }
    while (end != std::string::npos){
        result.push_back(str.substr(start, end - start));
        result.push_back("/");
        start = end + 1;
        end = str.find(delimiter, start);
    }
    if (start < str.length()) {
        result.push_back(str.substr(start));
    }
    return result;
}


Route *ParseRequest::findLocation(void){
    std::string copyRequest;

    this->_copyRequest = this->_headerInfo.url;
    std::vector<std::string> rootToken = doSplit(this->_headerInfo.url, '/');
    std::map<std::string, Route *> tmp = this->_server->getConfig()->getRoutes();
    std::map<std::string, Route*>::iterator it;
    it = tmp.end();
    while (it == tmp.end()){
        it = tmp.find(this->_copyRequest);
        if (it != tmp.end() && it->first == this->_copyRequest){
            break ;
        }
        if (rootToken.size() <= 0)
            break ;
        std::string tmp;
        for (size_t i = 0; i < rootToken.size() - 1; ++i)
            tmp += rootToken[i];
        this->_copyRequest = tmp;
        this->_saveRequest.insert(0, rootToken[rootToken.size() - 1]);
        rootToken.pop_back();
    }
    if (it == tmp.end())
        return (NULL);
    return (it->second);
}

int ParseRequest::initFinalPath(Route *route)
{
    std::string tmp;
    tmp = route->getRoot();
    if (tmp.size() > 1 && tmp.at(0) == '/')
        tmp.erase(0, 1);
    if (tmp.size() == 1 && tmp.at(0) == '/')
        tmp.insert(0, ".");
    if (route->getCgiAccept())
    {
        size_t pos = _saveRequest.find(".php");
        if (pos != std::string::npos)
        {
            if (pos + 4 < this->_saveRequest.size())
            {
                this->_pathInfo = _saveRequest.substr(pos + 4);
                this->_saveRequest.erase(pos + 4);
            }
        }
    }
    if (this->_saveRequest.size() != 0 && this->_saveRequest.at(0) != '/')
        this->_saveRequest.insert(0, "/");
    this->_finalPath = tmp + this->_saveRequest;
    std::cout << "[URL]" << this->_finalPath << std::endl;
    int value = findTypePath(this->_finalPath);
    if (value == 2)
    {
        std::cout << "ERROR 404 : Path not found" << std::endl;
        return (404);
    }
    else if (value == 3)
    {
        std::cout << "ERROR 403 : Path forbidden" << std::endl;
        return (403);
    }
    if (value == 4 && !route->getIndex().empty())
    {
        if (this->_finalPath[this->_finalPath.size() - 1] == '/')
            this->_finalPath += route->getIndex();
        else
            this->_finalPath += "/" + route->getIndex();
        value = findTypePath(this->_finalPath);
        if (value == 2)
        {
            std::cout << "ERROR 404 : Path not found" << std::endl;
            return (404);
        }
        else if (value == 3)
        {
            std::cout << "ERROR 403 : Path forbidden" << std::endl;
            return (403);
        }
    }
    else if (value == 4)
        this->_isDirectory = true;
    else if (value != 5)
    {
        std::cout << "ERROR 404 : Path not found" << std::endl;
        return (404);
    }
    return (0);
}

std::string ParseRequest::getHeader(void) const
{
    return (this->_header);
}

std::string ParseRequest::getBody(void) const
{
    return (this->_body);
}

bool ParseRequest::getIsDirectory(void) const
{
    return (this->_isDirectory);
}

Route   *ParseRequest::getRoute(void) const
{
    return (this->_route);
}

std::string ParseRequest::getBuffer(void) const
{
    return (this->_buffer);
}

int ParseRequest::getContentLength(void) const
{
    return (this->_headerInfo.contentLength);
}

int ParseRequest::getBytes(void) const
{
    return (this->_bytesRead);
}

t_header    ParseRequest::getHeaderInfo(void) const
{
    return (this->_headerInfo);
}

Server  *ParseRequest::getServer(void) const
{
    return (this->_server);
}

bool ParseRequest::getStartBloc(void) const
{
    return (this->_startBloc);
}

int ParseRequest::getSizeBloc(void) const
{
    return (this->_sizeBloc);
}

int ParseRequest::getReadBloc(void) const
{
    return (this->_readBloc);
}

Client  *ParseRequest::getClient(void) const
{
    return (this->_client);
}

std::string ParseRequest::getSaveRequest(void) const
{
    return (this->_saveRequest);
}

std::string ParseRequest::getFinalPath(void) const
{
    return (this->_finalPath);
}

std::string ParseRequest::getQueryString(void) const
{
    return (this->_queryString);
}

std::string ParseRequest::getPathInfo(void) const
{
    return (this->_pathInfo);
}

void ParseRequest::setBytes(int value)
{
    this->_bytesRead = value;
}

void ParseRequest::setContentLength(int value)
{
    this->_headerInfo.contentLength = value;
}

void ParseRequest::setHeader(std::string header)
{
    this->_header = header;
}

void ParseRequest::setBody(std::string body)
{
    this->_body = body;
}

void ParseRequest::setTransferEncoding(bool val)
{
    this->_headerInfo.transferEncoding = val;
}

void ParseRequest::setStartBloc(bool val)
{
    this->_startBloc = val;
}

void ParseRequest::setSizeBloc(int val)
{
    this->_sizeBloc = val;
}

void ParseRequest::setReadBloc(int val)
{
    this->_readBloc = val;
}

void ParseRequest::setBuffer(std::string buffer)
{
    this->_buffer = buffer;
}

void ParseRequest::setHasContentLength(bool val)
{
    this->_hasContentLength = val;
}