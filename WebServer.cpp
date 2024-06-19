#include "WebServer.hpp"

WebServer::WebServer(const std::vector<Server> &parsedServers) : _servers(parsedServers){

}

WebServer::~WebServer() {
    if(!this->_servers.empty()) {
        for (std::vector<Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it) {
            const std::vector<int>& server_fd_list = it->get_sock_fd();
            for (std::vector<int>::const_iterator fdIt = server_fd_list.begin(); fdIt != server_fd_list.end(); ++fdIt){
                close(*fdIt);
            }
        }
    }
}

void WebServer::nonBlocking(const int &fd) {
    int flags;
    int status;

    flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        throw std::runtime_error("Error: fcntl()");

    flags |= O_NONBLOCK;
    status = fcntl(fd, F_SETFL, flags);
    if(status == -1)
        throw std::runtime_error("Error: fcntl() status");
}

void WebServer::addToEpoll(const int &fd) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(this->_epollFD, EPOLL_CTL_ADD, fd, &event) == -1)
        throw std::runtime_error("epoll_ctl() failure");
}

void WebServer::addToEpollServers(){
    for (std::vector<Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
    {
        const std::vector<int>& server_fd_list = it->get_sock_fd();
        for (std::vector<int>::const_iterator fdIt = server_fd_list.begin(); fdIt != server_fd_list.end(); ++fdIt){
            if(*fdIt != -1){
                addToEpoll(*fdIt);
            }
        }
    }
}

int WebServer::isServerFDCheck(const int &i) const {
    for (std::vector<Server>::const_iterator it = this->_servers.begin(); it != this->_servers.end(); ++it) {
        const std::vector<int>& server_fd_list = it->get_sock_fd();
        for (std::vector<int>::const_iterator fdIt = server_fd_list.begin(); fdIt != server_fd_list.end(); ++fdIt){
            if (i == *fdIt)
                return (*fdIt);
        }
    }
    return (-1);
}

void WebServer::handleConnections() {
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
                    nonBlocking(newSockFD);
                    addToEpoll (newSockFD);
                }
            }
            else
            {
                int done = 0;

                while (true)
                {
                    memset(buf, 0, BUF_SIZE);
                    ssize_t bread = read(events[i].data.fd, buf, BUF_SIZE);

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
                    ResponseBuilder response(events[i].data.fd, this->_servers, buf);
                    response.buildResponse();
                    std::vector<char> responseString = response.getResponse();
                    write(events[i].data.fd, responseString.data(), responseString.size());
                }
                if (done)
                    close(events[i].data.fd);
            }
        }
    }
}

void WebServer::run() {
    for (std::vector<Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
    {
        const std::vector<int>& server_fd_list = it->get_sock_fd();
        const std::vector<Listen>& server_listeners = it->get_listeners();
        std::vector<Listen>::const_iterator listenIt = server_listeners.begin();
        for (std::vector<int>::const_iterator fdIt = server_fd_list.begin(); fdIt != server_fd_list.end(); fdIt++, listenIt++){
            if(*fdIt == -1)
                continue;

            nonBlocking(*fdIt);

            if(listen(*fdIt, SOMAXCONN) == -1) {
                std::string message = "liste() failure in: " + listenIt->host + ":" + listenIt->port;
                Logger::log(LOG_WARNING, message);
                continue ;
            }
            std::string message = "Server is listening on: " + listenIt->host + ":" + listenIt->port;
            Logger::log(LOG_INFO, message);
        }
    }

    this->_epollFD = epoll_create1(0);
    if (this->_epollFD == -1)
        throw std::runtime_error("epoll_create() failure");

    addToEpollServers();
    handleConnections();
}
