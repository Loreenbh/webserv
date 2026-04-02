#include "Client.hpp"
#include "utils.hpp"

/**
 * @brief Constructor for the Client class.
 * @details This constructor initializes a Client object with a reference to a Server object.
 *         It sets the file descriptor to -1, initializes the parse request pointer to NULL,
 *        and sets the content length to -1.
 * @param serv A reference to the Server object associated with this client.
 * @note This constructor is used to create a client instance that will interact with the server.
 * @throw None
 */
Client::Client(Server &serv) : _serv(serv),
									_fd(),
									_parseReq(NULL)
{
	this->_contentLength = -1;
	this->_buffer = "";
	this->_bytes = 0;
}


/**
 * @brief Destructor for the Client class.
 * @details This destructor cleans up resources used by the Client instance, including deleting the ParseRequest object if it exists.
 * @note This destructor is called when the Client instance goes out of scope or is deleted.
 */
Client::~Client()
{
	if (this->_parseReq)
	{  
		delete this->_parseReq;
		this->_parseReq = NULL;
	}
}

/**
 * @brief Disconnects the client from the server.
 * @details This method removes the client from the epoll instance and closes the file descriptor.
 *          It also prints a message indicating that the client has disconnected.
 * @note This method is called when the client disconnects from the server.
 */
void Client::disconnect(void)
{
	epoll_ctl(this->_serv.getEpollFd(), EPOLL_CTL_DEL, this->_fd, NULL);
	std::cout << "[Server " << this->_serv.getFd() << "] [Client " << this->_fd << "] disconnected" << std::endl;
	close(this->_fd);
}

/**
 * @brief Creates a new ParseRequest object for the client.
 * @details This method initializes a new ParseRequest object with the provided buffer and bytes,
 *          associating it with the server and the client.
 * @param buffer The request buffer to be parsed.
 * @param bytes The number of bytes in the buffer.
 * @note This method is called when a new request is received from the client.
 */
void Client::newParseRequest(std::string buffer, ssize_t bytes)
{
	this->_parseReq = new ParseRequest(buffer, bytes, &this->_serv, this);
}


/**
 * @brief Updates the existing ParseRequest object with new data.
 * @details This method adds the received buffer data to the existing ParseRequest object,
 *          either as part of the header or the body, depending on whether the header is already set.
 * @param recBuff The received buffer data.
 * @param bytes The number of bytes in the received buffer.
 * @note This method is called when additional data is received from the client.
 */
void Client::updateParseRequest(std::string recBuff, ssize_t bytes)
{
	if (this->_parseReq)
	{
		if (this->_parseReq->getHeader().empty())
			this->_parseReq->addBuffer(recBuff, bytes);
		else
			this->_parseReq->addBody(recBuff, bytes);
	}
}

/**
 * @brief Deletes the existing ParseRequest object.
 * @details This method deallocates the memory used by the ParseRequest object and sets the pointer to NULL.
 * @note This method is called when the request has been fully parsed or when the client disconnects.
 */
void Client::deleteParseRequest()
{
	if (this->_parseReq)
	{
		delete this->_parseReq;
		this->_parseReq = NULL;
	}
}

/**
 * @brief Parses the request header and body.
 * @details This method checks if the ParseRequest object is initialized and then calls its parseHeader method.
 *          If the transfer encoding is set, it also parses the body.
 * @return Returns the result of the parseHeader method, which indicates the status of the parsing.
 * @note This method is called to process the request after it has been received.
 */
int Client::parseRequest(void)
{
	if (!this->_parseReq)
	{
		std::cerr << "ParseRequest is NULL, cannot parse request" << std::endl;
		return (403);//
	}
	if (this->_parseReq->getHeaderInfo().transferEncoding)
		this->_parseReq->parseBody();
	return (this->_parseReq->parseHeader());
}


/**
 * @brief Splits the request buffer into header and body.
 * @details This method takes the position of the body in the request buffer and splits the buffer
 *          into header and body parts, updating the ParseRequest object accordingly.
 * @param body The position in the buffer where the body starts.
 * @note This method is called when the request has been fully received and needs to be parsed.
 */
void    Client::splitBuff(int body){
	if (!this->_parseReq)
	{
		std::cerr << "ParseRequest is NULL, cannot parse request" << std::endl;
		return;//
	}
	this->_parseReq->setHeader(this->_parseReq->getBuffer().substr(0, body - 1));
	this->_parseReq->setBody(this->_parseReq->getBuffer().substr(body));
}

/**
 * @brief Checks if the request is ready to be processed.
 * @details This method checks if the ParseRequest object is initialized and if the request has enough data
 *          to be parsed. It handles both content length and transfer encoding cases.
 * @return Returns the result of the parseRequest method if the request is ready, otherwise returns -1.
 * @note This method is called to determine if the request can be processed further.
 */
int Client::requestReady(void)
{
	if (!this->_parseReq)
	{
		std::cerr << "ParseRequest is NULL, cannot parse request" << std::endl;
		return (403);//
	}
	std::string buff = this->_parseReq->getBuffer();
	if (this->_parseReq->getContentLength() == -1 && !this->_parseReq->getHeaderInfo().transferEncoding)
	{
		size_t startBody = buff.find("\r\n\r\n");
		if (startBody != std::string::npos)
		{
			this->splitBuff(startBody + 4);
			size_t posContentLength;
			if ((posContentLength = this->_parseReq->getHeader().find("Content-Length:")) != std::string::npos)
			{
				posContentLength += 15;
				size_t end = this->_parseReq->getHeader().find("\r\n", posContentLength);
				std::stringstream ss(this->_parseReq->getHeader().substr(posContentLength, end - posContentLength));
				int nb;
				ss >> nb;
				this->_parseReq->setHasContentLength(true);
				this->_parseReq->setContentLength(nb);
				this->_parseReq->setBytes(this->_parseReq->getBytes() - startBody);
			}
			else if ((posContentLength = this->_parseReq->getHeader().find("Transfer-Encoding:")) != std::string::npos)
				this->_parseReq->setTransferEncoding(true);
			else
				return (this->parseRequest());
		}
	}
	if (this->_parseReq && this->_parseReq->getContentLength() != -1 && this->_parseReq->getContentLength() <= this->_parseReq->getBytes()){
		this->_parseReq->setBytes(this->_parseReq->getBytes() - 4);
		return this->parseRequest();
	}
	else if (this->_parseReq && this->_parseReq->getHeaderInfo().transferEncoding &&
		this->_parseReq->getBody().find("0\r\n\r\n") != std::string::npos){
			return (this->parseRequest());
	}
	return (-1);
}


void Client::clearHistoric(void)
{
	this->_historic.resize(0);
}

void Client::setFd(int fd)
{
	this->_fd = fd;
	struct epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = this->_fd;
	if (epoll_ctl(this->_serv.getEpollFd(), EPOLL_CTL_ADD, this->_fd, &event) == -1)
	{
		strerror(errno);
		throw Client::ErrorClient();
	}
}


/*
	Methode qui envoie la reponse au client.
*/
std::string Client::buildResponseDescription(std::string version, int code) const
{
	std::string codeStr = toString(code);
	return (version + " " + codeStr + " " + this->_serv.getStatusDescription(codeStr) + "\r\n");
}
void Client::buildResponseContentLength(int length)
{
	this->_responseHeaders["Content-Length"] = toString(length);
}
void Client::buildResponseConnection(std::string connection)
{
	this->_responseHeaders["Connection"] = connection;
}
void Client::buildResponseAcceptRanges(std::string acceptRanges)
{
	this->_responseHeaders["Accept-Ranges"] = acceptRanges;
}

void Client::buildResponseContentEncoding(std::string encoding)
{

	this->_responseHeaders["Content-Encoding"] = encoding;
}
void Client::buildResponseContentLocation(std::string location, int isReturn)
{
	if (isReturn)
		this->_responseHeaders["Location"] = location;
	else
		this->_responseHeaders["Content-Location"] = location;
}
void Client::buildResponseContentLanguage(std::string language)
{
	this->_responseHeaders["Content-Language"] = language;
}
void Client::buildResponseContentType(Request *req)
{
	std::string type;
	std::map<std::string, std::string> typeMime = req->getTypeMime();
	std::map<std::string, std::string>::iterator it = typeMime.find(req->getFileExt());
	if (it != typeMime.end())
		type = it->second;
	else
		type = "unknown";
	this->_responseHeaders["Content-Type"] = type;
}
std::string Client::buildHttpHeaders() {
	std::ostringstream stream;

	std::map<std::string, std::string>::const_iterator it;
	for (it = this->_responseHeaders.begin(); it != this->_responseHeaders.end(); ++it) {
		const std::string& key = it->first;
		const std::string& value = it->second;
		stream << key << ": " << value << "\r\n";
	}

	// Fin des headers HTTP
	stream << "\r\n";

	return stream.str();
}

void parseHttpHeaders(const std::string& rawHeaders, std::map<std::string, std::string>& headerMap) {
	std::istringstream stream(rawHeaders);
	std::string line;

	while (std::getline(stream, line)) {
		// Supprimer \r à la fin si présent
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		// Ignorer les lignes vides
		if (line.empty()) continue;

		// Trouver le premier ':'
		std::string::size_type colonPos = line.find(':');
		if (colonPos == std::string::npos) {
			continue; // ligne invalide, pas de ":"
		}

		std::string key = line.substr(0, colonPos);
		if (key == "Content-type")
			key = "Content-Type";
		if (key == "Content-length")
			key = "Content-Length";
		if (key == "Content-location")
			key = "Content-Location";
		if (key == "Content-encoding")
			key = "Content-Encoding";
		if (key == "Content-language")
			key = "Content-Language";
		std::string value = line.substr(colonPos + 1);

		// Supprimer les espaces au début de value
		while (!value.empty() && value[0] == ' ') {
			value.erase(0, 1);
		}

		headerMap[key] = value;
	}
}

bool Client::buildResponse(std::string htmlContent, int isReturn, int code, Request *req)
{
	size_t pos;
	std::string header;
	std::string body;
	this->_response = "";
	pos = htmlContent.find("Status:");
	if (pos != std::string::npos)
	{
		pos += 7;
		size_t end = htmlContent.find("\r\n", pos);
		std::string statusCode = htmlContent.substr(pos, end - pos);
		code = toInt(statusCode);
	}
	this->_responseHeaders.clear();
	this->_response = this->buildResponseDescription(req->getHeaderInfo().version, code);
	this->buildResponseContentType(req); //pardefaut text/html
	this->buildResponseConnection("keep-alive");
	this->buildResponseAcceptRanges("bytes");
	this->buildResponseContentEncoding("");
	this->buildResponseContentLanguage("en-US, fr-FR");
	this->buildResponseContentLocation(req->getHeaderInfo().url, isReturn);
	pos = htmlContent.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		header = htmlContent.substr(0, pos);
		body = htmlContent.substr(pos + 4);
		parseHttpHeaders(header, this->_responseHeaders);
	}
	else
	{        
		body = htmlContent;
		body += "\r\n";
	}
	this->buildResponseContentLength(body.size());
	this->_response += buildHttpHeaders();
	this->_response += body;
	struct epoll_event event = {};
	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = this->_fd;
	if (epoll_ctl(this->_serv.getEpollFd(), EPOLL_CTL_MOD, this->_fd, &event) == -1)
	{
		strerror(errno);
		return (false);
	}
	return (true);
}

bool Client::sendResponse(std::string htmlContent, int isReturn, int code, Request *req){

	size_t pos;
	std::string header;
	std::string body;
	std::string response = "";
	pos = htmlContent.find("Status:");
	if (pos != std::string::npos)
	{
		pos += 7;
		size_t end = htmlContent.find("\r\n", pos);
		std::string statusCode = htmlContent.substr(pos, end - pos);
		code = toInt(statusCode);
	}
	this->_responseHeaders.clear();
	response = this->buildResponseDescription(req->getHeaderInfo().version, code);
	this->buildResponseContentType(req); //pardefaut text/html
	this->buildResponseConnection("keep-alive");
	this->buildResponseAcceptRanges("bytes");
	this->buildResponseContentEncoding("");
	this->buildResponseContentLanguage("en-US, fr-FR");
	this->buildResponseContentLocation(req->getHeaderInfo().url, isReturn);
	pos = htmlContent.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		header = htmlContent.substr(0, pos);
		body = htmlContent.substr(pos + 4);
		parseHttpHeaders(header, this->_responseHeaders);
	}
	else
	{        
		body = htmlContent;
		body += "\r\n";
	}
	this->buildResponseContentLength(body.size());
	response += buildHttpHeaders();
		//std::cerr << "Response : " << std::endl << response << "===================" << std::endl;
	response += body;
		//std::cerr << "Response : " << std::endl << response << "===================" << std::endl;
	ssize_t bytes_sent = send(this->getFd(), response.c_str(), response.size(), 0);
	if (bytes_sent < 0) {
		strerror(errno);
		this->disconnect();
		return (false);
	}
	return (true);
}



std::string Client::genFile(int code){
	std::string htmlContent;
	htmlContent += "<html><head><title> ERROR " + toString(code) + "</title>";
	htmlContent += "<meta charset=\"UTF-8\">";
	htmlContent += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
	htmlContent += "<style>body{background-color: #f0f0f0; font-family: Arial, sans-serif;}</style>";
	htmlContent += "</head>";
	htmlContent += "<body>";
	htmlContent += "<h1> Error " + toString(code) + "</h1>";
	htmlContent += "<p>" + this->_serv.getStatusDescription(toString(code)) + "</p>";
	htmlContent += "</body></html>";
	return (htmlContent);
}

bool		Client::buildErrorPage(int code)
{
	this->_response = "";
	std::map<int, std::string> errorPage = this->_serv.getConfig()->getErrorPages();
	std::map<int, std::string>::iterator it = errorPage.begin();
	for(; it != errorPage.end(); ++it){
		if (it->first == code)
			break;
	}
	std::string htmlContent;
	if (it == errorPage.end() || !readFile(it->second))
	{
		this->_responseBody =  genFile(code);
	}
	// build th header
	{
		std::string convertCode = toString(code);
		std::string description = this->_serv.getStatusDescription(convertCode);
		ssize_t content_length = this->_responseBody.size();
		std::string content_string = toString(content_length);
		this->_response = "HTTP/1.1 " + convertCode + " " + description + "\r\n";
		
		this->_response += "Content-Type: text/html\r\n";
		this->_response +=
		"Connection: keep-alive\r\n"
		"Content-Length: " + content_string + "\r\n" + "\r\n";
	}
	struct epoll_event event = {};
	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = this->_fd;
	if (epoll_ctl(this->_serv.getEpollFd(), EPOLL_CTL_MOD, this->_fd, &event) == -1)
	{
		strerror(errno);
		return (false);
	}
	this->_response += this->_responseBody;
	return (true);
}

bool      Client::sendErrorPage(int code){
	std::string response;
	std::cerr << "Error page code : " << code << std::endl;
	std::map<int, std::string> errorPage = this->_serv.getConfig()->getErrorPages();
	std::map<int, std::string>::iterator it = errorPage.begin();
	for(; it != errorPage.end(); ++it){
		if (it->first == code)
			break;
	}
	std::string htmlContent;
	if (it == errorPage.end() || !readFile(it->second))
	{
		this->_responseBody =  genFile(code);
	}
	// build th header
	{
		std::string convertCode = toString(code);
		std::string description = this->_serv.getStatusDescription(convertCode);
		ssize_t content_length = this->_responseBody.size();
		//std::cerr << "Content length : " << content_length << std::endl;
		std::string content_string = toString(content_length);
		response = "HTTP/1.1 " + convertCode + " " + description + "\r\n";
		
		response += "Content-Type: text/html\r\n";
		response +=
		"Connection: keep-alive\r\n"
		"Content-Length: " + content_string + "\r\n" + "\r\n";
	}

	response += this->_responseBody;
	ssize_t bytes_sent = send(this->_fd, response.c_str(), response.size(), 0);
	if (bytes_sent < 0) {
		std::cerr << "SEND : " << strerror(errno) << std::endl;
		//this->disconnect();
		return (false);
	}
	return (true);
}

bool Client::readFile(std::string &path)
{
    if (path.empty())
        return (false);
    std::ifstream file(path.c_str());
    if (file.is_open())
    {
        std::ostringstream contentStream;
        contentStream << file.rdbuf();
        this->_responseBody = contentStream.str();
		file.close();
		return (true);
	}
	else
	{
		std::cerr << "ERROR open file: " << path << std::endl;
		return (false);
    }
    return (false);
}

struct sockaddr_in &Client::getAddr(void)
{
	return (this->_addr);
}

ParseRequest *Client::getParseReq(void) const
{
	return (this->_parseReq);
}

// GETTERS
int Client::getFd(void) const
{
	return (this->_fd);
}

std::vector<std::string> &Client::getHistoric(void)
{
	return (this->_historic);
}

std::string Client::getBuffer(void) const
{
	return (this->_buffer);
}

std::string	Client::getResponse(void) const
{
	return (this->_response);
}

// EXCEPTIONS
const char *Client::ErrorClient::what() const throw()
{
	return ("Error on constructor Client");
}