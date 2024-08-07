#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <sys/epoll.h>
# include <sys/socket.h>
# include <fcntl.h>

# include "../utils/Logger.hpp"
# include "../config/Server.hpp"
# include "../utils/defines.hpp"
# include "../http/ResponseBuilder.hpp"
# include "../http/RequestParser.hpp"
# include "../cgi/HandleCGI.hpp"
# include <csignal>


class Server;
class Logger;

class WebServer {
	private:
		int									_epollFD;
		std::map<int, std::vector<Server>>	_fdToServers;
		std::map<int, std::vector<Server>>	_conections;
		std::map<int, HandleCGI*>			_requestsCGI;
		std::map<int, std::string*>			_requests;
		static bool							isRunning;

		WebServer();

		static void	handleSignal(int param);

		Server		delegateServer(std::vector<Server> candidateServers, std::string host);

		void		creatingAndBinding(const std::map<std::string, std::vector<Server>> &groupServers);
		void		settingListeners( );
		void		addToEpollServers( );
		void		addToEpoll(const int &fd, uint32_t events);
		void		modifyEpoll(const int &fd, uint32_t events);
		void		acceptConnection(int *serverFd);
		void 		closeConnection(const int &fd, std::string message);
		void 		closeCGIPipe(const int &fd, std::string message);
		void 		clearCGIRequests(const int &fd);
		void 		clearRequests(const int &fd);
		void		sendErrorResponse(const int &statusCode, const int &fd, const Server &server);
		void		handleConnections( );

		int			isServerFDCheck(const int &i) const;
		int			ifResGetCGIKey(const int &fd);



	public:
		WebServer(const std::vector<Server> &parsedServers);
		~WebServer();

		void 		run();

};

#endif
