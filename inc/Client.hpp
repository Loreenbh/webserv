# ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Server.hpp"
# include "ParseRequest.hpp"
# include "Request.hpp"


# include <errno.h>
# include <string.h>
# include <map>
# include <vector>


class ParseRequest;
class Server;
class Request;

class Client
{
    public:
        Client(Server &serv);
        ~Client();
        void disconnect();
        void newParseRequest(std::string buffer, ssize_t bytes);
        void updateParseRequest(std::string buffer, ssize_t bytes);
        void deleteParseRequest(void);
        int  requestReady(void);
        void splitBuff(int body);
        int  parseRequest(void);
        bool sendErrorPage(int code);
        bool sendResponse(std::string htmlContent, int isReturn, int code, Request *req);
        std::string genFile(int code);
        bool buildResponse(std::string htmlContent, int isReturn, int code, Request *req);
        bool buildErrorPage(int code);
        
        void setFd(int fd);
        // GETTERS
        std::string getResponse(void) const;
        int getFd(void) const;
        std::string getBuffer(void) const;
        ParseRequest *getParseReq(void) const;
        void        clearHistoric(void);
        std::vector<std::string> &getHistoric(void);
        struct sockaddr_in &getAddr(void);

    private:
        Server&									_serv;
        int										_fd;
        std::string								_buffer;
        ssize_t									_bytes;
        struct sockaddr_in						_addr;
        int 									_contentLength;
        ParseRequest*							_parseReq;
        std::vector<std::string>				_historic;
        std::map<std::string, std::string>		_responseHeaders;
		std::string								_responseBody;
        std::string                             _response;
    
        std::string buildResponseDescription(std::string version, int code) const;
        void buildResponseConnection(std::string connection);
        void buildResponseAcceptRanges(std::string acceptRanges);
        void buildResponseContentEncoding(std::string encoding);
        void buildResponseContentLanguage(std::string language);
        void buildResponseContentLocation(std::string location, int isReturn);
        void buildResponseContentType(Request *req);
        void buildResponseContentLength(int length);

        // Build the response
        std::string buildHttpHeaders();
        bool 		readFile(std::string &path);
    class ErrorClient : public std::exception
    {
        public:
            virtual const char *what() const throw();
    };
};

#endif