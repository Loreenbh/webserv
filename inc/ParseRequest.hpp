/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseRequest.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 10:43:02 by glions            #+#    #+#             */
/*   Updated: 2025/05/28 23:38:37 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSEREQUEST_HPP
# define PARSEREQUEST_HPP

# include "Server.hpp"
# include "Request.hpp"
# include "utils.hpp"



class Server;
class Client;
class Route;

class ParseRequest
{
	public:
		ParseRequest(std::string buffer, ssize_t bytes, Server *serv, Client *client);
		~ParseRequest();
		int                         parseHost(std::string host);
		int                         parseRequest(std::string request);
		int                         parseHeader(void);
		void                        addBuffer(std::string buffer, ssize_t bytes);
		void                        addBody(std::string buffer, ssize_t bytes);
		Route                      *findLocation(void);
		void                        cutQueryString(void);
		int                         initFinalPath(Route *route);
		void                        parseBody(void);

		void                        setBytes(int value);
		void                        setContentLength(int value);
		void                        setTransferEncoding(bool val);
		void                        setStartBloc(bool val);
		void                        setSizeBloc(int val);
		void                        setReadBloc(int val);
		void                        setHeader(std::string header);
		void                        setBody(std::string body);
		void                        setBuffer(std::string buffer);
		void                        setHasContentLength(bool val);

		void                        cutPathInfo(void);
		std::string                 getBuffer(void) const;
		int                         getContentLength(void) const;
		int                         getBytes(void) const;
		std::string                 getHeader(void) const;
		t_header                    getHeaderInfo(void) const;
		std::string                 getBody(void) const;
		Route                       *getRoute(void) const;
		Server                      *getServer(void) const;
		Client                      *getClient(void) const;
		bool                        getIsDirectory(void) const;
		std::string                 getSaveRequest(void) const;
		std::string                 getFinalPath(void) const;
		std::string                 getPathInfo(void) const;
		std::string                 getQueryString(void) const;
		bool                        getStartBloc(void) const;
		int                         getSizeBloc(void) const;
		int                         getReadBloc(void) const;

	private:
		t_header    _headerInfo;
		std::string _header;
		std::string _body;
		std::string _buffer;
		int         _bytesRead;
		std::string _copyRequest;
		std::string _saveRequest;
		std::string _finalPath;
		std::string _pathInfo;
		std::string _queryString;
		bool        _isDirectory;
		bool        _startBloc;
		bool        _hasContentLength;
		int         _sizeBloc;
		int         _readBloc;
		Route       *_route;
		Server      *_server;
		Client      *_client;
		std::string             _memBuff;
};

#endif