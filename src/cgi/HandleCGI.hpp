#ifndef HANDLECGI_HPP
# define HANDLECGI_HPP

# include "../../defines.hpp"
# include "../../Server.hpp"
# include "../../Location.hpp"
# include "../ResponseBuilder.hpp"
# include <vector>
# include <string>
# include <iostream>
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/epoll.h>
# include <fcntl.h>
# include <string.h>
# include <cstdlib>
# include <cstdio>
# include <cstring>

class HandleCGI {
	private:
		httpRequest                 _request;
		std::vector<std::string>    _env;
        int                         _fdEpool;

        bool        getCGILocation(Location &location);
        std::string getFullCGIPath();
        std::string getCGIPath();
        bool        isDirectory(std::string path);
        bool        isFile(std::string path);
		char        **buildEnv();
		char	    **convertEnv();
        void	    freeEnv(char **env);
        void        sendErrorResponse(int statusCode, int fd);
        bool        checkResponse();
        std::string getCGIHeaders();
        std::string getCGIBody();
        std::string getMethod(int method);

	public:
		HandleCGI();
		~HandleCGI();
		HandleCGI(httpRequest parsedRequest, int &fdEpool, int responseFd, Server server);

		int         executeCGI();
        Response    getCGIResponse();

        int         _pipefd[2];
        int         _responseFd;
        bool        responseReady;
        Server      _server;
		std::string	_responseCGI;

};

#endif