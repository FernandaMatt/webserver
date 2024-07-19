#include "WebServer.hpp"

bool WebServer::isRunning = false;

WebServer::WebServer(const std::vector<Server> &parsedServers) {
	std::map<std::string, std::vector<Server>> groupServers;

	std::vector<Server>::const_iterator it = parsedServers.begin();
	for( it; it != parsedServers.end(); ++it)
	{
		const std::vector<Listen> &listeners = it->get_listeners();
		std::vector<Listen>::const_iterator listenIt = listeners.begin();
		for (listenIt; listenIt != listeners.end(); ++listenIt)
		{
			std::string key = listenIt->host + ":" +listenIt->port;
			groupServers[key].push_back(*it);
		}
	}

	creatingAndBinding(groupServers);
	settingListeners();

	isRunning = true;
}

WebServer::~WebServer() {
	if (isRunning)
		isRunning = false;

	std::map<int, HandleCGI*>::iterator itCGI = _requestsCGI.begin();
	for (itCGI; itCGI != _requestsCGI.end(); ++itCGI)
	{
		epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, itCGI->second->_pipefd[0], 0);
		close(itCGI->second->_pipefd[0]);
		delete itCGI->second;
	}
	this->_requestsCGI.clear();

	std::map<int, std::string*>::iterator itReq = _requests.begin();
	for (itReq; itReq != _requests.end(); ++itReq)
	{
		delete itReq->second;
	}
	this->_requests.clear();

	std::map<int, std::vector<Server>>::iterator conit = this->_conections.begin();
	for (conit; conit != this->_conections.end(); ++conit)
	{
		epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, conit->first, 0);
		close(conit->first);
	}
	this->_conections.clear();

	std::map<int, std::vector<Server>>::const_iterator it = this->_fdToServers.begin();
	for (it; it != this->_fdToServers.end(); ++it)
	{
		epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, it->first, 0);
		close(it->first);
	}
	this->_fdToServers.clear();

	close (this->_epollFD);
}

void WebServer::handleSignal(int param) {
	(void)param;
	isRunning = false;
	Logger::log(LOG_INFO, "Signal received, stopping server...");
}

Server WebServer::delegateRequest(std::vector<Server> candidateServers, std::string host) {
	for (std::vector<Server>::iterator itServer = candidateServers.begin(); itServer != candidateServers.end(); ++itServer) {
		std::vector<std::string> servernames = itServer->get_server_name();
		for (std::vector<std::string>::iterator itServerName = servernames.begin(); itServerName != servernames.end(); ++itServerName) {
			if (*itServerName == host) {
				return *itServer;
			}
		}
	}
	return candidateServers[0];
}

void WebServer::creatingAndBinding(const std::map<std::string, std::vector<Server>> &groupServers)
{
	std::map<std::string, std::vector<Server>>::const_iterator mapIt = groupServers.begin();
	for (mapIt; mapIt != groupServers.end(); ++mapIt)
	{
		size_t lastColonsPos = mapIt->first.find_last_of(":");
		std::string host = mapIt->first.substr(0, lastColonsPos);
		std::string port = mapIt->first.substr(lastColonsPos + 1);

		struct addrinfo	hints;
		struct addrinfo	*result;
		struct addrinfo	*rp;
		int				status;
		int				sock_fd = -1;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = 0;

		status = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
		if (status != 0)
		{
			Logger::log(LOG_ERROR, gai_strerror(status));
			throw std::runtime_error("Error: getaddrinfo()");
		}

		for (rp = result; rp != NULL; rp = rp->ai_next)
		{
			sock_fd = socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK , rp->ai_protocol);

			if (sock_fd == -1)
				continue;

			int option = 1;
			if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
			{
				Logger::log(LOG_ERROR, "setsockopt() failure");
				close(sock_fd);
				freeaddrinfo(result);
				throw std::runtime_error("Error: setsockopt()");
			}

			if (bind(sock_fd, rp->ai_addr, rp->ai_addrlen) == 0)
			{
				this->_fdToServers[sock_fd] = mapIt->second;
				std::vector<Server>::iterator it = _fdToServers[sock_fd].begin();
				for (it; it != _fdToServers[sock_fd].end(); ++it)
					(*it).set_host_port(mapIt->first);
				break;
			}
		}
		if (rp == NULL)
		{
			if(sock_fd != -1)
				close(sock_fd);
			Logger::log(LOG_ERROR, "Could not bind " + mapIt->first);
		}
		freeaddrinfo(result);
	}
}

void WebServer::settingListeners() {
	std::map<int, std::vector<Server>>::const_iterator it = this->_fdToServers.begin();
	for (it; it != this->_fdToServers.end(); ++it)
	{
		if (listen (it->first, SOMAXCONN) == -1)
		{
				std::string message = "listen() failure in: " + _fdToServers[it->first][0].get_host_port();
				Logger::log(LOG_WARNING, message);
				continue ;
		}
		std::string message = "Server is listening on: " + _fdToServers[it->first][0].get_host_port();
		Logger::log(LOG_INFO, message);
	}
}

void WebServer::addToEpoll(const int &fd, uint32_t events) {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(this->_epollFD, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		Logger::log(LOG_ERROR, "Failed to add server socket to epoll, closing connection fd: " + std::to_string(fd));
		close(fd);
	}
}

void WebServer::modifyEpoll(const int &fd, uint32_t events) {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(this->_epollFD, EPOLL_CTL_MOD, fd, &event) == -1)
	{
		Logger::log(LOG_ERROR, "Failed to modify server socket to epoll, closing connection fd: " + std::to_string(fd));
		close(fd);
		epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, fd, 0);
	}
}

void WebServer::addToEpollServers(){
	std::map<int, std::vector<Server>>::const_iterator it = this->_fdToServers.begin();
	for (it; it != this->_fdToServers.end(); ++it)
	{
		addToEpoll(it->first, EPOLLIN | EPOLLET);
	}
}

int WebServer::isServerFDCheck(const int &i) const {
	std::map<int, std::vector<Server>>::const_iterator it = this->_fdToServers.begin();
	for (it; it != this->_fdToServers.end(); ++it)
	{
		if ( i == it->first)
			return (it->first);
	}
	return (-1);
}

void WebServer::acceptConnection(int *serverFd)
{
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len = sizeof(peer_addr);

	int newSockFD = accept(*serverFd, (struct sockaddr *) &peer_addr, &peer_addr_len);
	if (newSockFD == -1)
	{
		Logger::log(LOG_WARNING, "accept() failure");
		return ;
	}

	//CHECAR COM A FERNANDA!!!!! Fernanda e Mari: Não deveria chear antes de abrir o socket com accept?
	// check if total connections is greater than SOMAXCONN
	int totalConnections = this->_conections.size() + this->_fdToServers.size() + 1000;
	if (totalConnections >= SOMAXCONN)
	{
		if (newSockFD != -1)
		{
			Logger::log(LOG_WARNING, "Max connections reached, closing connection...");
			Response response;
			response.loadDefaultErrorPage(503);
			close(newSockFD);
		}
		return ;
	}

	std::map<int, std::vector<Server>>::iterator it = this->_fdToServers.find(*serverFd);
	if (it != this->_fdToServers.end())
	{
        _conections[newSockFD] = it->second;
		_requests[newSockFD] = new std::string();
		modifyEpoll(it->first, EPOLLIN | EPOLLET);
	}
	else
	{
		Logger::log(LOG_ERROR, "Server socket fd not found!");
		Response response;
		response.loadDefaultErrorPage(503);
		close(newSockFD);
		return ;
	}

	addToEpoll (newSockFD, EPOLLIN | EPOLLRDHUP);
	std::ostringstream oss;
	oss << "Connection established between socket [" << *serverFd << "] and client [" << newSockFD << "]";
	Logger::log(LOG_INFO, oss.str().c_str());
}

void WebServer::closeConnection(const int &fd, std::string message)
{
	Logger::log(LOG_WARNING, message);
	epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, fd, 0);
	close(fd);
	if (_conections.find(fd) != _conections.end())
		_conections.erase(fd);
	clearRequests(fd);
}

void WebServer::closeCGIPipe(const int &fd, std::string message)
{
	Logger::log(LOG_WARNING, message);
	epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}

void WebServer::clearCGIRequests(const int &fd)
{
	if (_requestsCGI.find(fd) != _requestsCGI.end())
	{
		delete _requestsCGI[fd];
		_requestsCGI.erase(fd);
	}
}

void WebServer::clearRequests(const int &fd)
{
	if (_requests.find(fd) != _requests.end())
	{
		delete _requests[fd];
		_requests.erase(fd);
	}
}

int WebServer::ifResGetCGIKey(const int &fd)
{
	std::map<int, HandleCGI*>::const_iterator it = _requestsCGI.begin();
	for (it; it != _requestsCGI.end(); ++it)
	{
		if (it->second->_responseFd == fd)
			return (it->first);
	}
	return (-1);
}

void WebServer::handleConnections()
{
	struct epoll_event events[MAX_EVENTS];

	while (true)
	{
		int totalFD = epoll_wait(this->_epollFD, events, MAX_EVENTS, -1);

		if (!isRunning)
			break;

		if (totalFD == -1)
			throw std::runtime_error("epoll_wait() failure");


		for (int i = 0; i < totalFD; i++)
		{
			int fd = events[i].data.fd;

			int isServerFD = isServerFDCheck(events[i].data.fd);
			if (isServerFD != -1)
				acceptConnection(&isServerFD);
			else
			{
				if (events[i].events & EPOLLRDHUP)
				{
					closeConnection(fd, "Connection closed on the remote side (EPOLLRDHUP): " + std::to_string(fd));
				}
				else if (events[i].events & EPOLLIN)
				{
					//check if actual fd is in CGI map, if so, read from pipe
					if (_requestsCGI.find(fd) != _requestsCGI.end())
					{
						char buf[BUF_SIZE];
						memset(buf, 0, BUF_SIZE);
						HandleCGI &cgiH = *_requestsCGI[fd];

						while (true)
						{

							ssize_t bread = read(cgiH._pipefd[0], buf, BUF_SIZE);
							if (bread == 0)
							{
								closeCGIPipe(cgiH._pipefd[0], "EOF reached. Closing pipe fd: " + std::to_string(cgiH._pipefd[0]));
                                cgiH.responseReady = 1;
								break ;
							}
							else if (bread < 0)
							{
								closeCGIPipe(cgiH._pipefd[0], "Error reading from pipe. Closing pipe fd: " + std::to_string(cgiH._pipefd[0]));
								closeConnection(cgiH._responseFd, "Error reading from pipe. Closing respective connection: " + std::to_string(cgiH._responseFd));
								clearCGIRequests(fd);
								break ;
							}
							else
							{
								cgiH._responseCGI.append(buf, bread);
								memset(buf, 0, BUF_SIZE);
								if (bread < BUF_SIZE)
									break;
							}
						}
					}
					else //else read from socket connection and get request
					{
						char buf[BUF_SIZE];
						memset(buf, 0, BUF_SIZE);
						while (true)
						{
							ssize_t bread = read(fd, buf, BUF_SIZE);
							if (bread == 0)
							{
								closeConnection(fd, "Connection closed by client (FIN): " + std::to_string(fd));
								break ;
							}
							else if (bread < 0)
							{
								closeConnection(fd, "Error reading from client: " + std::to_string(fd));
								break ;
							}
							else
							{
								_requests[fd]->append(buf, bread);
								memset(buf, 0, BUF_SIZE);
								if (bread < BUF_SIZE)
									break;
							}
						}
						if (_requests.find(fd) != _requests.end())
						{
							httpRequest req = RequestParser::parseRequest(*_requests[fd]);
							if (req.request_status == "complete")
							{
								modifyEpoll(fd, EPOLLOUT );
							}
						}
					}
				}
				else if (events[i].events & EPOLLOUT)
				{
					//check if fd is in requestsCGI, if so, check if response is ready
					int cgiKey = ifResGetCGIKey(fd);
					if (cgiKey != -1)
					{
						if(_requestsCGI[cgiKey]->responseReady == 1)
						{
							Response responseCGI;
							responseCGI = _requestsCGI[cgiKey]->getCGIResponse();
							size_t wbytes = write(fd, responseCGI.getResponse().data(), responseCGI.getResponseSize());
							if (wbytes <= 0)
							{
								Logger::log(LOG_ERROR, "write() failure, response from CGI not sent.");
							}
							closeCGIPipe(cgiKey,"Closing pipe fd: " + std::to_string(cgiKey));
							clearCGIRequests(cgiKey);
							closeConnection(fd, "Closing respective connection: " + std::to_string(fd));
						}
					}
					else
					{
						//check if fd is in requests, if so, parse request
						if (_requests.find(fd) != _requests.end())
						{
							httpRequest req = RequestParser::parseRequest(*_requests[fd]);
							//check if request is complete and if so, check if it is CGI or STATIC
							if (req.request_status == "complete")
							{
								//check if request is CGI or STATIC
								if (req.type == "CGI")
								{
									Logger::log(LOG_WARNING, "CGI Request RECEIVED. Handling..." );

									HandleCGI *cgiHandler = new HandleCGI(req, this->_epollFD, events[i].data.fd, delegateRequest(_conections[events[i].data.fd], req.host));
									int fdPipe = cgiHandler->executeCGI();

									if (fdPipe == -1)
									{
										closeConnection(fd, "CGI script failed to execute. Closing connection: " + std::to_string(fd));
										delete cgiHandler;
									}
									else {
										_requestsCGI[fdPipe] = cgiHandler;
										continue;
									}
								}
								if (req.type == "STATIC")
								{
									ResponseBuilder response;
									response.buildResponse(delegateRequest(_conections[events[i].data.fd], req.host), req);
									std::vector<char> responseString = response.getResponse();
									size_t wbytes = write(events[i].data.fd, responseString.data(), responseString.size());
									if (wbytes == 0)
									{
										Logger::log(LOG_ERROR, "write() failure, response not sent.");
									}
									else if (wbytes < responseString.size())
									{
										Logger::log(LOG_ERROR, "write() failure, response not sent completely.");
									}
									else
									{
										Logger::log(LOG_INFO, "Response sent successfully.");
									}
										closeConnection(fd, "Closing connection: " + std::to_string(fd));
								}
							}
						}
					}
				}
                else if ((events[i].events & EPOLLHUP) && (_requestsCGI.find(fd) != _requestsCGI.end()))
                {
					char buf[BUF_SIZE];
					memset(buf, 0, BUF_SIZE);
                    HandleCGI &cgiH = *_requestsCGI[fd];
                    ssize_t bread = read(cgiH._pipefd[0], buf, BUF_SIZE);
                    if (bread == 0)
						cgiH.responseReady = 1;
                }
			}
		}
	}
}


void WebServer::run() {
	this->_epollFD = epoll_create1(0);
	if (this->_epollFD == -1)
		throw std::runtime_error("epoll_create() failure");

	addToEpollServers();
	signal(SIGINT, WebServer::handleSignal);
	handleConnections();
}
