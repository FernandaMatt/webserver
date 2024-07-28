# include "HandleCGI.hpp"

HandleCGI::HandleCGI() {};

HandleCGI::HandleCGI(httpRequest parsedRequest, int &fdEpool, int responseFd, Server server) {
	this->_request = parsedRequest;
    this->_fdEpool = fdEpool;
    this->_responseFd = responseFd;
	this->_responseCGI = "";
    this->_server = server;
    this->responseReady = false;
};

HandleCGI::~HandleCGI() {};

int HandleCGI::executeCGI() {

    if (!getCGILocation()) {
        sendErrorResponse(404, _responseFd);
        return -1;
    }

    std::size_t statusMethoBodySize = forbiddenMethodOrBodySize();

    if (statusMethoBodySize != 200) {
        sendErrorResponse(statusMethoBodySize, _responseFd);
        return -1;
    }

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
            std::perror("chdir");
            freeEnv(envp);
            free(path);
            free(script_file);
            sendErrorResponse(500, STDOUT_FILENO);
            exit(EXIT_FAILURE);
        }

		if (execve(path, argv, envp) == -1) {
			std::perror("execve");
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

    std::string cgi_path = _location.get_cgi_path();
    if(!_request.aliasPath.empty())
    {
        if (!_request.aliasPath.empty() && _request.aliasPath.at(_request.aliasPath.size() - 1) == '/')
            _request.aliasPath = _request.aliasPath.substr(0, _request.aliasPath.size() - 1);
        if (!_request.aliasPath.empty() && _request.aliasPath.at(0) == '/')
            _request.aliasPath = _request.aliasPath.substr(1);
        if (!cgi_path.empty() && cgi_path.at(cgi_path.size() - 1) == '/')
            cgi_path = cgi_path.substr(0, cgi_path.size() - 1);
        cgi_path += "/" + _request.aliasPath;
    }
    if (cgi_path.empty() || !isDirectory(cgi_path))
        return "";

    return cgi_path;
}

std::string HandleCGI::getFullCGIPath() {
    std::string cgi_path = getCGIPath();
    if (!cgi_path.empty() && cgi_path.at(cgi_path.size() -1) != '/')
        cgi_path += '/';
    std::string cgi_full_path = cgi_path + _request.CGIfilename;
    if (!isFile(cgi_full_path))
        return "";
    return cgi_full_path;
}

bool HandleCGI::getCGILocation() {
    std::vector<Location> locations = _server.get_location();
    std::string locationPath = _request.CGIpath;
    std::size_t pos = _request.CGIpath.find_last_of("/");

    while (pos != std::string::npos && locationPath.length() > 1)
    {
        for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); ++it) {
            if (locationPath == it->get_path()) {
                _location = *it;
                return true;
            }
        }
        locationPath = _request.CGIpath.substr(0, pos);
        _request.aliasPath = _request.CGIpath.substr(pos);
        pos = _request.CGIpath.find_last_of("/", pos - 1);

    }
    return false;
}

std::string HandleCGI::getPathTranslated() {
    std::string path_translated = _location.get_cgi_path();
    std::string path_info = _request.extraPath;
    if (!path_translated.empty() && path_translated.at(path_translated.size() - 1) != '/')
        path_translated += '/';
    if (!path_info.empty() && path_info.at(0) == '/')
        path_info = path_info.substr(1);
    path_translated += path_info;
    return path_translated;
}

char ** HandleCGI::buildEnv() {

    _env.clear();
    if (_request.body.size() > 0)
        _env.push_back("CONTENT_LENGTH=" + std::to_string(_request.body.size()));
    if (_request.headers.find("Content-Type") != _request.headers.end())
        _env.push_back("CONTENT_TYPE=" + _request.headers["Content-Type"]);
    _env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    _env.push_back("PATH_INFO=" + _request.extraPath);
    if (!_request.extraPath.empty())
        _env.push_back("PATH_TRANSLATED=" + getPathTranslated());
    _env.push_back("QUERY_STRING=" + _request.queryString);
    _env.push_back("REQUEST_METHOD=" + getMethod(_request.method));
    _env.push_back("SCRIPT_FILENAME=" + getFullCGIPath());
    _env.push_back("SCRIPT_NAME=" + _request.path);
    _env.push_back("SERVER_NAME=" + _request.host);
    _env.push_back("SERVER_PORT=" + _request.port);
    _env.push_back("SERVER_PROTOCOL=" + _request.version);
    _env.push_back("SERVER_SOFTWARE=webserv/1.0");
    _env.push_back("REDIRECT_STATUS=200");

    for (std::map<std::string, std::string>::iterator it = _request.headers.begin(); it != _request.headers.end(); it++)
        _env.push_back("HTTP_" + it->first + "=" + it->second);

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
    if (write(_responseFd, responseContent.data(), responseContent.size()) <= 0)
        Logger::log(LOG_ERROR, "write() from HandleCGI::sendoErrorResponse failed");
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

    std::map<std::string, std::string> headers = parseCGIHeaders();
    std::string statusLine = headers["Status"];
    if (statusLine.empty()) {
        response.setStatusMessage("HTTP/1.1 200 OK\r\n");
    }
    else {
        response.setStatusMessage("HTTP/1.1 " + statusLine + "\r\n");
    }
    response.setHttpHeaders(getCGIHeaders());
    response.setResponseContent(getCGIBody());

    return response;
}

std::map<std::string, std::string> HandleCGI::parseCGIHeaders()
{
    std::map<std::string, std::string> headers;
    std::string header = getCGIHeaders();
    size_t pos = 0;
    size_t end = header.find("\r\n", pos);
    while (end != std::string::npos && end != pos) {
        std::string line = header.substr(pos, end);
        size_t colon = line.find(":");
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2);
            while (value.size() && (value.at(value.size() -1) == '\n' || value.size() && value.at(value.size() -1) == '\r'))
                value = value.substr(0, value.size() - 1);
            headers[key] = value;
        }
        pos = end + 2;
        end = header.find("\r\n", pos);
    }
    return headers;
}

std::string HandleCGI::getCGIHeaders() {
    size_t header_end = _responseCGI.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return "";
    return _responseCGI.substr(0, header_end + 4);
}

std::string HandleCGI::getCGIBody() {
    size_t header_end = _responseCGI.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return "";
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

bool HandleCGI::isMethodAllowed(const std::string &method) {
    if (std::find(_location.get_methods().begin(), _location.get_methods().end(), method) == _location.get_methods().end()) {
        return false;
    }
    return true;
}

bool HandleCGI::isBodySizeAllowed() {
    if (_request.body.length() > _location.get_client_max_body_size()) {
        return false;
    }
    return true;
}

int HandleCGI::forbiddenMethodOrBodySize() {
    std::string method = "OTHER";
    if (_request.method == GET) {
       method = "GET";
    }
    else if (_request.method == POST) {
        method = "POST";
    }
    else if (_request.method == DELETE) {
        method = "DELETE";
    }
    if (!isMethodAllowed(method)) {
        return 405;
    }
    if ((method == "GET" || method == "POST") && !isBodySizeAllowed()) {
        return 413;
    }
    return 200;
}