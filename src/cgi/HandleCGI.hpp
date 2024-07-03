#ifndef HANDLECGI_HPP
# define HANDLECGI_HPP

# include "../../defines.hpp"
# include <vector>
# include <string>
# include <iostream>
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>

class HandleCGI {
	private:
		httpRequest _request;
		std::vector<std::string> _env;

		std::vector<std::string>	buildEnv();
		char **						convertEnv();

	public:
		HandleCGI();
		~HandleCGI();
		HandleCGI(httpRequest parsedRequest);

		void executeCGI();
		std::string executeTest();

};

#endif
