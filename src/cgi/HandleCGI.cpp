# include "HandleCGI.hpp"

HandleCGI::HandleCGI() {};

HandleCGI::HandleCGI(httpRequest parsedRequest, int &fdEpool, int responseFd) {
	this->_request = parsedRequest;
    this->_fdEpool = fdEpool;
    this->_responseFd = responseFd;
};

HandleCGI::~HandleCGI() {};

int HandleCGI::executeTest() {
	std::cout << "Executing test" << std::endl;

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
		//close
		close(_pipefd[0]);

		//dup2
		dup2(_pipefd[1], STDOUT_FILENO);

		// Execute the program
		if (execve(path, argv, envp) == -1) {
			perror("execve");
			exit(EXIT_FAILURE);
		}

	} else {
		//close
		// close(_pipefd[1]);

        struct epoll_event event;
        event.data.fd = _pipefd[1];
        event.events = EPOLLOUT;
        if (epoll_ctl(_fdEpool, EPOLL_CTL_ADD, _pipefd[1], &event) == -1)
            throw std::runtime_error("epoll_ctl() failure");
		//read

		// waitpid(pid, NULL, 0);
		// char buffer[300];
		// int n = read(pipefd[0], buffer, 300);
		// buffer[n] = '\0';

		// close(pipefd[0]);
		return _pipefd[1];
	}
	return -1;
}
