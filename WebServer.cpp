#include "WebServer.hpp"

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

	// // just to ckeck REMOVE IT
	// std::map<std::string, std::vector<Server>>::const_iterator checkIt = groupServers.begin();
	// for ( checkIt; checkIt != groupServers.end(); ++checkIt)
	// {
	// 	std::cout << "for key: " << checkIt->first << std::endl;
	// 	int count = 1;
	// 	std::vector<Server>::const_iterator servIt = checkIt->second.begin();
	// 	for (servIt; servIt != checkIt->second.end(); ++servIt)
	// 	{
	// 		std::cout << "server[" << count << "]:" << std::endl;
	// 		(*servIt).print_all_directives();
	// 		count++;
	// 	}
	// }
	// // just to ckeck REMOVE IT

	creatingAndBinding(groupServers);

	// // just to ckeck REMOVE IT
	// std::map<int, std::vector<Server>>::const_iterator fdIT = this->_fdToServers.begin();
	// for ( fdIT; fdIT != this->_fdToServers.end(); ++fdIT)
	// {
	// 	std::cout << "for fd: " << fdIT->first << std::endl;
	// 	int count = 1;
	// 	std::vector<Server>::const_iterator servIt = fdIT->second.begin();
	// 	for (servIt; servIt != fdIT->second.end(); ++servIt)
	// 	{
	// 		std::cout << "server[" << count << "]:" << std::endl;
	// 		(*servIt).print_all_directives();
	// 		count++;
	// 	}
	// }
	// // just to ckeck REMOVE IT
}

WebServer::~WebServer() {
	std::map<int, std::vector<Server>>::const_iterator it = this->_fdToServers.begin();
	for (it; it != this->_fdToServers.end(); ++it)
	{
		close(it->first);
	}
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

void WebServer::addToEpoll(const int &fd) {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(this->_epollFD, EPOLL_CTL_ADD, fd, &event) == -1)
		throw std::runtime_error("epoll_ctl() failure");
}

void WebServer::addToEpollServers(){
	std::map<int, std::vector<Server>>::const_iterator it = this->_fdToServers.begin();
	for (it; it != this->_fdToServers.end(); ++it)
	{
		addToEpoll(it->first);
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

void WebServer::handleConnections()
{
	struct epoll_event events[MAX_EVENTS];
	char buf[BUF_SIZE];

	while (true)
	{
		int totalFD = epoll_wait(this->_epollFD, events, MAX_EVENTS, -1);
		if (totalFD == -1)
			throw std::runtime_error("epoll_wait() failure");

		for (int i = 0; i < totalFD; i++)
		{
			int isServerFD = isServerFDCheck(events[i].data.fd);
			if (isServerFD != -1)
			{
				while (true)
				{
					struct sockaddr_storage peer_addr;
					socklen_t peer_addr_len = sizeof(peer_addr);

					int newSockFD = accept(isServerFD, (struct sockaddr *) &peer_addr, &peer_addr_len);
					if (newSockFD == -1)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK )
							break ;
						else
						{
							Logger::log(LOG_WARNING, "accept() failure");
							break;
						}
					}
					addToEpoll (newSockFD);
				}
			}
			else
			{
				int done = 0;

				while (true)
				{
					ssize_t bread = read(events[i].data.fd, buf, sizeof(buf));
					if (bread == -1)
					{
						if (errno != EAGAIN)
						{
							Logger::log(LOG_WARNING, "read() failure");
							done = 1;
						}
						break ;
					}
					else if (bread == 0)
					{
						done = 1;
						break ;
					}
					std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
					write(events[i].data.fd, response.c_str(), response.length());
				}
				if (done)
				{
					epoll_ctl(this->_epollFD, EPOLL_CTL_DEL, events[i].data.fd, 0);
					close(events[i].data.fd);
				}
			}
		}
	}
}

void WebServer::run() {
	settingListeners();

	this->_epollFD = epoll_create1(0);
	if (this->_epollFD == -1)
		throw std::runtime_error("epoll_create() failure");

	addToEpollServers();
	handleConnections();
}
