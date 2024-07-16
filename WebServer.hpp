#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <sys/epoll.h>
# include <sys/socket.h>
# include <fcntl.h>

# include "Logger.hpp"
# include "Server.hpp"
# include "defines.hpp"
# include "src/ResponseBuilder.hpp"
# include "src/RequestParser.hpp"
# include "src/cgi/HandleCGI.hpp"
# include <csignal>


class Server;
class Logger;

class WebServer {
	private:
		int					_epollFD;
		std::map<int, std::vector<Server>> _fdToServers;
		std::map<int, std::vector<Server>> _conections;
		std::map<int, HandleCGI*> _requestsCGI;
		std::map<int, std::string*> _requests;
		static bool				isRunning;

		WebServer();

		void	creatingAndBinding(const std::map<std::string, std::vector<Server>> &groupServers);
		void	settingListeners();
		void	addToEpollServers( );
		void	addToEpoll(const int &fd, uint32_t events);
		void	acceptConnection(int *serverFd);
		void	handleConnections( );
		void 	closeConnection(const int &fd, std::string message);
		void 	closeCGIPipe(const int &fd, std::string message);
		void 	clearCGIRequests(const int &fd);
		void 	clearRequests(const int &fd);
		int		ifResGetCGIKey(const int &fd);

		int		isServerFDCheck(const int &i) const;
		Server	delegateRequest(std::vector<Server> candidateServers, std::string host);

		static void	handleSignal(int param);

	public:
		WebServer(const std::vector<Server> &parsedServers);
		~WebServer();

		void run();

};

#endif
