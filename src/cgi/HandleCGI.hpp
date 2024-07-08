#ifndef HANDLECGI_HPP
# define HANDLECGI_HPP

# include "../../defines.hpp"
# include <vector>
# include <string>
# include <iostream>
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/epoll.h>
# include <fcntl.h>
# include <string.h>

class HandleCGI {
	private:
		httpRequest _request;
		std::vector<std::string> _env;

		char    **buildEnv();
        int     _fdEpool;

	public:
		HandleCGI();
		~HandleCGI();
		HandleCGI(httpRequest parsedRequest, int &fdEpool, int responseFd);

		// void        executeCGI();
		int         executeCGI();
		char		**convertEnv();
        void		freeEnv(char **env);
        int         _pipefd[2];
        int         _responseFd;
		std::string	_responseCGI;

};

#endif
