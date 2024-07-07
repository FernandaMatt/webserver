# include "HandleCGI.hpp"

HandleCGI::HandleCGI() {};

HandleCGI::HandleCGI(httpRequest parsedRequest, int &fdEpool, int responseFd) {
	this->_request = parsedRequest;
    this->_fdEpool = fdEpool;
    this->_responseFd = responseFd;
	this->_responseCGI = "";
};

HandleCGI::~HandleCGI() {};

int HandleCGI::executeTest() {

	const char *path = "/bin/teste-cgi/test.php";

    // Arguments for execve
    // The first argument should be the name of the executable itself
    char *const argv[] = { (char *)path, NULL };

    // Environment variables for execve (can be NULL if not needed)
    char *const envp[] = { NULL };

	//criar pipe
	if (pipe(_pipefd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	//fork
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
        // int pipeBody[2];

		close(_pipefd[0]);

		dup2(_pipefd[1], STDOUT_FILENO);

		close(_pipefd[1]);

        // if (pipe(pipeBody) == -1) {
        //     perror("pipe");
        //     exit(EXIT_FAILURE);
        // }

        // dup2(pipeBody[0], STDIN_FILENO);

        // if (write(pipeBody[1], _request.body.c_str(), _request.body.size()) <= 0)
        //     throw std::runtime_error("write() failure");

        // close(pipeBody[1]);

        // close(pipeBody[0]);

		if (execve(path, argv, envp) == -1) {
			perror("execve");
			exit(EXIT_FAILURE);
		}

	} else {
		close(_pipefd[1]);

        struct epoll_event event;
        event.data.fd = _pipefd[0];
        event.events = EPOLLIN ;
        if (epoll_ctl(_fdEpool, EPOLL_CTL_ADD, _pipefd[0], &event) == -1)
            throw std::runtime_error("epoll_ctl() failure");

		return _pipefd[0];
	}
	return -1;
}

std::vector<std::string> HandleCGI::buildEnv() {
    std::vector<std::string> env;

    if (_request.body.size() > 0)
        env.push_back("CONTENT_LENGTH=" + std::to_string(_request.body.size()));
    if (_request.headers.find("Content-Type") != _request.headers.end())
        env.push_back("CONTENT_TYPE=" + _request.headers["Content-Type"]);
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");


    //print env
    for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); ++it)
        std::cout << *it << std::endl;
    return env;
}
