# include "HandleCGI.hpp"

HandleCGI::HandleCGI() {};

HandleCGI::HandleCGI(httpRequest parsedRequest, int &fdEpool, int responseFd, Server server) {
	this->_request = parsedRequest;
    this->_fdEpool = fdEpool;
    this->_responseFd = responseFd;
	this->_responseCGI = "";
    this->_server = server;
};

HandleCGI::~HandleCGI() {};

int HandleCGI::executeCGI() {

    std::string cgi_full_path = getFullCGIPath();

    if (cgi_full_path.empty()) {
        sendErrorResponse(404, _responseFd);
        return -1;
    }

	if (pipe(_pipefd) == -1) {
		Logger::log(LOG_ERROR, "pipe() failed");
        sendErrorResponse(500, _responseFd);
		return -1;
	}


	pid_t pid = fork();
	if (pid == -1) {
        Logger::log(LOG_ERROR, "fork() failed");
        close(_pipefd[0]);
        close(_pipefd[1]);
        sendErrorResponse(500, _responseFd);
        return -1;
	}

	if (pid == 0) {

        int pipeBody[2];

        if (pipe(pipeBody) != -1) {
            dup2(pipeBody[0], STDIN_FILENO);
            close(pipeBody[0]);
            Logger::log(LOG_INFO, "Request body: " + _request.body);
            if (_request.body.size() > 0) {
                if (write(pipeBody[1], _request.body.c_str(), _request.body.size()) <= 0)
                    Logger::log(LOG_ERROR, "write() failed");
                else
                    Logger::log(LOG_INFO, "Request body sent to CGI");
            }
            close(pipeBody[1]);
        }
        else {
            Logger::log(LOG_ERROR, "Failed to create pipe for body");
            sendErrorResponse(500, STDOUT_FILENO);
            close(_pipefd[0]);
            close(_pipefd[1]);
            exit(EXIT_FAILURE);
        }

        char *path = strdup(cgi_full_path.c_str());

        char *script_file = strdup(_request.CGIfilename.c_str());

        char *const argv[] = { (char *)script_file, NULL };

        char **envp = buildEnv();


		close(_pipefd[0]);

		dup2(_pipefd[1], STDOUT_FILENO);

		close(_pipefd[1]);

        //change directory to cgi path
        std::string cgi_path = getCGIPath();
        if (chdir(cgi_path.c_str()) == -1) {
            perror("chdir");
            freeEnv(envp);
            free(path);
            free(script_file);
            sendErrorResponse(500, STDOUT_FILENO);
            exit(EXIT_FAILURE);
        }

		if (execve(path, argv, envp) == -1) {
			perror("execve");
            freeEnv(envp);
            free(path);
            free(script_file);
            sendErrorResponse(500, STDOUT_FILENO);
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

std::string HandleCGI::getCGIPath() {
    Location location;
    if (!getCGILocation(location))
        return "";

    std::string cgi_path = location.get_cgi_path();
    if (cgi_path.empty() || !isDirectory(cgi_path))
        return "";

    return cgi_path;
}

std::string HandleCGI::getFullCGIPath() {
    std::string cgi_path = getCGIPath();
    if (cgi_path.back() != '/')
        cgi_path += '/';
    std::string cgi_full_path = cgi_path + _request.CGIfilename;
    if (!isFile(cgi_full_path))
        return "";
    return cgi_full_path;
}

bool HandleCGI::getCGILocation(Location &location) {
    std::vector<Location> locations = _server.get_location();
    for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); ++it) {
        if (_request.path == it->get_path()) {
            location = *it;
            return true;
        }
    }
    return false;
}

char ** HandleCGI::buildEnv() {

    if (_request.body.size() > 0)
        _env.push_back("CONTENT_LENGTH=" + std::to_string(_request.body.size()));
        _env.push_back("CONTENT_TYPE=");
    if (_request.headers.find("Content-Type") != _request.headers.end())
        _env.push_back("CONTENT_TYPE=" + _request.headers["Content-Type"]);
    _env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    _env.push_back("PATH_INFO=" + _request.extraPath);
    //PATH_TRANSLATED ??
    _env.push_back("QUERY_STRING=" + _request.queryString);
    //REMOTE_ADDR ??
    _env.push_back("REQUEST_METHOD=" + getMethod(_request.method));
    _env.push_back("SCRIPT_NAME=" + _request.path);
    _env.push_back("SERVER_NAME=" + _request.host);
    _env.push_back("SERVER_PORT=" + _request.port);
    _env.push_back("SERVER_PROTOCOL=" + _request.version);
    _env.push_back("SERVER_SOFTWARE=webserv/1.0");
    for (std::map<std::string, std::string>::iterator it = _request.headers.begin(); it != _request.headers.end(); it++)
        _env.push_back("HTTP_" + it->first + "=" + it->second);


    //print _env
    for (std::vector<std::string>::iterator it = _env.begin(); it != _env.end(); ++it)
        std::cout << *it << std::endl;
    
    char **env_char = convertEnv();
    return env_char;
}

char ** HandleCGI::convertEnv() {
    char **env = new char*[this->_env.size() + 1];
    for (size_t i = 0; i < this->_env.size(); i++) {
        env[i] = strdup(this->_env[i].c_str());
    }
    env[this->_env.size()] = NULL;
    return env;
}

void HandleCGI::freeEnv(char **env) {
    for (size_t i = 0; env[i] != NULL; i++) {
        free(env[i]);
    }
    delete[] env;
}

void HandleCGI::sendErrorResponse(int statusCode, int fd) {
    Response response;

    response.loadErrorPage(statusCode, _server, false);

    std::vector<char> responseContent;
    responseContent = response.getResponse();
    write(_responseFd, responseContent.data(), responseContent.size());
}

bool HandleCGI::isDirectory(std::string path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0) {
        return false;
    }
    return S_ISDIR(pathStat.st_mode);
}

bool HandleCGI::isFile(std::string path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0) {
        return false;
    }
    return S_ISREG(pathStat.st_mode);
}

Response HandleCGI::getCGIResponse() {
    Response response;

    if (!checkResponse())
        response.loadErrorPage(500, _server, false);
    else {
        response.setStatusMessage("HTTP/1.1 200 OK\r\n");
        response.setHttpHeaders(getCGIHeaders());
        response.setResponseContent(getCGIBody());
    }

    return response;
}

bool HandleCGI::checkResponse() {
    size_t pos = _responseCGI.find("Content-Type:");
    size_t header_end = _responseCGI.find("\r\n\r\n");
    if (header_end == std::string::npos)
        header_end = _responseCGI.find("\n\n");
    if (pos == std::string::npos || pos > header_end)
        return false;
    return true;
}

std::string HandleCGI::getCGIHeaders() {
    size_t header_end = _responseCGI.find("\r\n\r\n");
    return _responseCGI.substr(0, header_end + 4);
}

std::string HandleCGI::getCGIBody() {
    size_t header_end = _responseCGI.find("\r\n\r\n");
    return _responseCGI.substr(header_end + 4);
}

std::string HandleCGI::getMethod(int method) {
    if (method == GET)
        return "GET";
    else if (method == POST)
        return "POST";
    else if (method == DELETE)
        return "DELETE";
    return "";
}