# include "HandleCGI.hpp"

HandleCGI::HandleCGI() {};

HandleCGI::HandleCGI(httpRequest parsedRequest) {
	this->_request = parsedRequest;
};

HandleCGI::~HandleCGI() {};

std::string HandleCGI::executeTest() {
	std::cout << "Executing test" << std::endl;

	const char *path = "/bin/teste-cgi/test.php";

    // Arguments for execve
    // The first argument should be the name of the executable itself
    char *const argv[] = { (char *)path, NULL };

    // Environment variables for execve (can be NULL if not needed)
    char *const envp[] = { NULL };

	//criar pipe
	int pipefd[2];

	if (pipe(pipefd) == -1) {
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
		close(pipefd[0]);

		//dup2
		dup2(pipefd[1], STDOUT_FILENO);

		// Execute the program
		if (execve(path, argv, envp) == -1) {
			perror("execve");
			exit(EXIT_FAILURE);
		}

	} else {
		//close
		close(pipefd[1]);

		//read
		waitpid(pid, NULL, 0);
		char buffer[300];
		int n = read(pipefd[0], buffer, 300);
		buffer[n] = '\0';

		close(pipefd[0]);
		return std::string(buffer);
	}
	return "";
}
