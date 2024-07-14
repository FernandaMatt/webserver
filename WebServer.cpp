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
		throw std::runtime_error("epoll_ctl() failure");
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
	int totalConnections = this->_conections.size() + this->_fdToServers.size() + 8;
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
        _conections[newSockFD] = it->second;
	else
	{
		Logger::log(LOG_ERROR, "Server socket fd not found!");
		Response response;
		response.loadDefaultErrorPage(503);
		close(newSockFD);
		return ;
	}

	addToEpoll (newSockFD, EPOLLIN | EPOLLOUT);
	std::ostringstream oss;
	oss << "Connection established between socket [" << *serverFd << "] and client [" << newSockFD << "]";
	Logger::log(LOG_INFO, oss.str().c_str());
}

void WebServer::handleConnections()
{
	struct epoll_event events[MAX_EVENTS];

    ResponseBuilder response;

	while (true)
	{
		int totalFD = epoll_wait(this->_epollFD, events, MAX_EVENTS, -1);

		if (!isRunning)
			break;

		if (totalFD == -1)
			throw std::runtime_error("epoll_wait() failure");


		for (int i = 0; i < totalFD; i++)
		{
			int done = 0;

			int isServerFD = isServerFDCheck(events[i].data.fd);
			if (isServerFD != -1)
				acceptConnection(&isServerFD);
			else
			{
				int fd = events[i].data.fd;
				char buf[BUF_SIZE];
				memset(buf, 0, BUF_SIZE);
				std::string request;
				// checking for EPOLLIN event, ready to read from fd
				if (events[i].events & EPOLLIN)
				{
					//check if actual fd is in CGI map, if so, read from pipe
					if (_requestsCGI.find(fd) != _requestsCGI.end())
					{
						HandleCGI &cgiH = *_requestsCGI[fd];

						while (true)
						{
							ssize_t bread = read(cgiH._pipefd[0], buf, BUF_SIZE);
							if (bread <= 0)
							{
								if (bread == 0)
								{
                                    Logger::log(LOG_INFO, "Done reading from pipe cgi " + std::to_string(cgiH._pipefd[0]) + ". EOF reached.");
                                    epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, cgiH._pipefd[0], 0);
                                    close(cgiH._pipefd[0]);
                                    cgiH.responseReady = 1;
								}
                                else {
									Logger::log(LOG_INFO, "Something is wrong from reading pipe cgi " + std::to_string(cgiH._pipefd[0]));
								// if bread <= 0, that means an error occurred, remove from epoll, close fds, remove from maps
                                    epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, cgiH._pipefd[0], 0);
                                    close(cgiH._pipefd[0]);
                                    epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, cgiH._responseFd, 0);
                                    close(cgiH._responseFd);
                                    _conections.erase(cgiH._responseFd);
                                    cgiH._responseCGI = "";
                                    delete _requestsCGI[fd];
                                    _requestsCGI.erase(fd);
                                    break ;
                                }
							}
							cgiH._responseCGI.append(buf, bread);
							memset(buf, 0, BUF_SIZE);
							if (bread < BUF_SIZE)
								break;
						}
					}
					else //else read from socket connection and get request
					{
						while (true)
						{
							ssize_t bread = read(fd, buf, BUF_SIZE);
							if (bread <= 0)
							{
								if (bread == 0)
								{
									Logger::log(LOG_INFO, "Nothing to read, clossing connection: " + std::to_string(fd));
								}
								// if bread <=0 , that means an error occurred, so done is set to close the connection and remove from epoll
								done = 1;
								break ; //with error only break the loop for now
							}
							request.append(buf, bread);
							memset(buf, 0, BUF_SIZE);
							// if bread < BUF_SIZE, that means everything was read, so break the loop
							if (bread < BUF_SIZE)
								break;
						}
					}
				}
				if (events[i].events & EPOLLOUT)
				{
					//check if request is not empty
					if (request.size() > 0)
					{
						httpRequest req = RequestParser::parseRequest(request);
						//check if request is incomplete or empty
						if (req.request_status == "incomplete" || req.request_status == "")
						{
							//if req is incomplete, check if requests is already in _requests map, if so, append it
							if (_requests.find(fd) != _requests.end())
							{
								_requests[fd]->append(request);
								httpRequest tmp = RequestParser::parseRequest(*_requests[fd]);
								req = tmp;
							}
							//else add request to _requests map
							else
							{
								std::string *addReq = new std::string(request);
								_requests[fd] = addReq;
							}
						}
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
                                    Logger::log(LOG_WARNING, "CGI script failed to execute. Error response sent. Closing connection: " + std::to_string(events[i].data.fd));
                                    done = 1;
                                }
                                else {
                                    _requestsCGI[fdPipe] = cgiHandler;
                                    continue;
                                }
						    }
                        }
						if (req.type == "STATIC")
						{
							response.buildResponse(delegateRequest(_conections[events[i].data.fd], req.host), req);
							std::vector<char> responseString = response.getResponse();
							size_t wbytes = write(events[i].data.fd, responseString.data(), responseString.size());
							if (wbytes <= 0)
							{
								if (wbytes == -1)
									Logger::log(LOG_ERROR, "write() failure, response not sent, closing connection: " + std::to_string(events[i].data.fd));
							}
							done = 1; //answer sent, close connection
						}
					}
					//check if fd is in responseFd, if so, send response
					std::map<int, HandleCGI*>::iterator it = _requestsCGI.begin();
					for(it; it != _requestsCGI.end(); ++it)
					{
						if (it->second->_responseFd == events[i].data.fd)
						{
							if (it->second->_responseCGI.size() > 0 && it->second->responseReady == 1)
							{
                                Response response;
                                response = it->second->getCGIResponse();
                                char * test = strdup(response.getResponse().data()); //apagar
                                size_t testsize = response.getResponseSize(); //apagar
								size_t wbytes = write(it->second->_responseFd, response.getResponse().data(), response.getResponseSize());
								if (wbytes <= 0)
								{
									if(wbytes == -1)
										Logger::log(LOG_ERROR, "write() failure, response not sent, closing connection: " + std::to_string(it->second->_responseFd));
								}
								delete _requestsCGI[it->first];
								_requestsCGI.erase(it->first);
								done = 1;
							}
							break;
						}
					}
				}
                if ((events[i].events & EPOLLHUP) && (_requestsCGI.find(fd) != _requestsCGI.end()))
                {
                    HandleCGI &cgiH = *_requestsCGI[fd];
                    ssize_t bread = read(cgiH._pipefd[0], buf, BUF_SIZE);
                    if (bread == 0){
                        Logger::log(LOG_INFO, "Closing pipe fd: " + std::to_string(cgiH._pipefd[0]));
                        epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, cgiH._pipefd[0], 0);
                        close(cgiH._pipefd[0]);
                        cgiH.responseReady = 1;
                    }
                }
				if (done)
				{
					Logger::log(LOG_INFO, "Closing connection: " + std::to_string(events[i].data.fd));
					epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, events[i].data.fd, 0);
                    _conections.erase(events[i].data.fd);
					close(events[i].data.fd);
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
