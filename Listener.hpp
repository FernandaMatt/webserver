#ifndef LISTENER_HPP
# define LISTENER_HPP

# include <iostream>
# include "Logger.hpp"
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <unistd.h>
# include <stdexcept>
# include <string>
# include <cstring>
# include <errno.h>

class Listener {
  public:
       Listener(std::string const &host, std::string const &port);
       ~Listener(void);
        bool listen(void) const;

		int                         sock_fd;
        const std::string           port;
		const std::string           host;
		struct sockaddr_storage     addr_info;
};
#endif // !LISTENER_HPP