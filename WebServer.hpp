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
		int					_epollFD;
		std::map<int, std::vector<Server>> _fdToServers;

		WebServer();

		void	creatingAndBinding(const std::map<std::string, std::vector<Server>> &groupServers);
		void	settingListeners();
		void	addToEpollServers( );
		void	addToEpoll(const int &fd, uint32_t events);
		void	acceptConnection(int *serverFd, ResponseBuilder response);
		void	handleConnections( );
		int		isServerFDCheck(const int &i) const;

	public:
		WebServer(const std::vector<Server> &parsedServers);
		~WebServer();

		void run();

};

#endif
