// #include "WebServer.hpp"

// WebServer::WebServer(const std::vector<Server> &parsedServers) : _servers(parsedServers){

// }

// WebServer::~WebServer() {
// 	if(!this->_servers.empty()) {
// 		for (std::vector<Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
// 			close(it->get_sock_fd());
// 	}
// }

// void WebServer::nonBlocking(const int &fd) {
// 	int	flags;
// 	int	status;

// 	flags = fcntl(fd, F_GETFL, 0);
// 	if(flags == -1)
// 		throw std::runtime_error("Error: fcntl()");

// 	flags |= O_NONBLOCK;
// 	status = fcntl(fd, F_SETFL, flags);
// 	if(status == -1)
// 		throw std::runtime_error("Error: fcntl() status");
// }

// void WebServer::addToEpoll(const int &fd) {
// 	struct epoll_event event;
// 	event.data.fd = fd;
// 	event.events = EPOLLIN | EPOLLET;
// 	if (epoll_ctl(this->_epollFD, EPOLL_CTL_ADD, fd, &event) == -1)
// 		throw std::runtime_error("epoll_ctl() failure");
// }

// void WebServer::addToEpollServers(){
// 	for (std::vector<Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
// 	{
// 		if (it->get_sock_fd() != -1)
// 			addToEpoll(it->get_sock_fd());
// 	}
// }

// int WebServer::isServerFDCheck(const int &i) const {
// 	for (std::vector<Server>::const_iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
// 	{
// 		if (i == it->get_sock_fd())
// 			return (it->get_sock_fd());
// 	}
// 	return (-1);
// }

// void WebServer::handleConnections() {
// 	struct epoll_event events[MAX_EVENTS];
// 	char buf[BUF_SIZE];

// 	while (true)
// 	{
// 		int totalFD = epoll_wait(this->_epollFD, events, MAX_EVENTS, -1);
// 		if (totalFD == -1)
// 			throw std::runtime_error("epoll_wait() failure");

// 		for (int i = 0; i < totalFD; i++)
// 		{
// 			int isServerFD = isServerFDCheck(events[i].data.fd);
// 			if (isServerFD != -1)
// 			{
// 				while (true)
// 				{
// 					struct sockaddr_storage peer_addr;
// 					socklen_t peer_addr_len = sizeof(peer_addr);

// 					int newSockFD = accept(isServerFD, (struct sockaddr *) &peer_addr, &peer_addr_len);
// 					if (newSockFD == -1)
// 					{
// 						if (errno == EAGAIN || errno == EWOULDBLOCK )
// 							break ;
// 						else
// 						{
// 							Logger::log(LOG_WARNING, "accept() failure");
// 							break;
// 						}
// 					}
// 					nonBlocking(newSockFD);
// 					addToEpoll (newSockFD);
// 				}
// 			}
// 			else
// 			{
// 				int done = 0;

// 				while (true)
// 				{
// 					ssize_t bread = read(events[i].data.fd, buf, sizeof(buf));
// 					if (bread == -1)
// 					{
// 						if (errno != EAGAIN)
// 						{
// 							Logger::log(LOG_WARNING, "read() failure");
// 							done = 1;
// 						}
// 						break ;
// 					}
// 					else if (bread == 0)
// 					{
// 						done = 1;
// 						break ;
// 					}
// 					std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
// 					write(events[i].data.fd, response.c_str(), response.length());
// 				}
// 				if (done)
// 					close(events[i].data.fd);
// 			}
// 		}
// 	}
// }

// void WebServer::run() {
// 	for (std::vector<Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
// 	{
// 		if (it->get_sock_fd() == -1)
// 			continue ;

// 		nonBlocking(it->get_sock_fd());

// 		if(listen(it->get_sock_fd(), SOMAXCONN) == -1) {
// 			std::string message = "liste() failure in: " + it->get_host() + ":" + it->get_port();
// 			Logger::log(LOG_WARNING, message);
// 			continue ;
// 		}
// 		std::string message = "Server is listening on: " + it->get_host() + ":" + it->get_port();
// 		Logger::log(LOG_INFO, message);
// 	}

// 	this->_epollFD = epoll_create1(0);
// 	if (this->_epollFD == -1)
// 		throw std::runtime_error("epoll_create() failure");

// 	addToEpollServers();
// 	handleConnections();
// }
