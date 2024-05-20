#include "Listener.hpp"

Listener::Listener(std::string const &host, std::string const &port) : host(host), port(port), sock_fd(-1) {

    struct addrinfo hint;
    struct addrinfo *result;
    int yes = 1;
    int error;

    hint = {0};
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = 0;

    if ((error = getaddrinfo(host.c_str(), port.c_str(), &hint, &result)) != 0) {
        Logger::log(LOG_ERROR, gai_strerror(error));
        throw std::runtime_error("Error");
    }

    switch (result->ai_family) {
    case AF_INET:
        memmove(&this->addr_info, result->ai_addr, sizeof(struct sockaddr_in));
        break;
    case AF_INET6:
        memmove(&this->addr_info, result->ai_addr, sizeof(struct sockaddr_in6));
        break;
    default:
        Logger::log(LOG_ERROR, "Uncaught error");
        throw std::runtime_error("Error");
    }

    if ((this->sock_fd = socket(result->ai_family, result->ai_socktype | SOCK_CLOEXEC | SOCK_NONBLOCK, 0)) == -1) {
        Logger::log(LOG_ERROR, "Error in socket");
        freeaddrinfo(result);
        throw std::runtime_error("Error");
    }

    if ((setsockopt(this->sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
        Logger::log(LOG_ERROR, "Error in setsockopt");
        freeaddrinfo(result);
        throw std::runtime_error("Error");
    }

    if (bind(this->sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
        Logger::log(LOG_ERROR, strerror(errno));
        freeaddrinfo(result);
        throw std::runtime_error("Error");
    }
    freeaddrinfo(result);
}

bool Listener::listen(void) const {
    std::string hostPort;
    hostPort = this->host + ":" + this->port;

    if (::listen(this->sock_fd, SOMAXCONN) == -1) {
      Logger::log(LOG_ERROR, (std::string("Listen failed for pair ") + hostPort).c_str());
        throw std::runtime_error("Error");
	}
    Logger::log(LOG_INFO, (std::string("Listening on ") + hostPort).c_str());
    return(true);
}

Listener::~Listener(void) {
    if (this->sock_fd != -1) {
        close(this->sock_fd);
    }
}