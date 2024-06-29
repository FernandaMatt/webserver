#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <sys/epoll.h>
# include <sys/socket.h>
# include <fcntl.h>

# include "Logger.hpp"
# include "Server.hpp"
# include "defines.hpp"
# include "src/ResponseBuilder.hpp"

class Server;
class Logger;

class WebServer {
	private:
		std::vector<Server>	_servers;
		int					_epollFD;

		WebServer();

		void	nonBlocking(const int &fd);
		void	addToEpollServers( );
		void	addToEpoll(const int &fd);
		void	handleConnections( );
		int		isServerFDCheck(const int &i) const;

	public:
		WebServer(const std::vector<Server> &parsedServers);
		~WebServer();

		void run();

};

#endif
