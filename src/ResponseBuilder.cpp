#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder() {
}

ResponseBuilder::ResponseBuilder(int fd, std::vector<Server> servers, char *request) : 
	_fd(fd), _candidateServers(servers), _request(request) {
	RequestParser parser;
	_parsedRequest = parser.parseRequest(request); //turn into static function and in the buildResponse function
    _response = "HTTP/1.1 200 OK\r\nContent-Length: 39\r\n\r\nDefault Response from Response Builder!";
}

ResponseBuilder::~ResponseBuilder() {
}

void ResponseBuilder::printInitializedAttributes() {
	std::cout << GRN <<"fd: " << RST << _fd << std::endl;
	std::cout << GRN << "servers: " << RST << std::endl;
	for (std::vector<Server>::iterator it = _candidateServers.begin(); it != _candidateServers.end(); ++it) {
		it->print_all_directives();
		std::cout << BLUE <<"-------------" << RST << std::endl ;
	}
	std::cout << GRN << "request: " << RST << _request << std::endl;
	std::cout << GRN << "parsedRequest: " << RST << std::endl;
	_parsedRequest.printRequest();
}

void ResponseBuilder::delegateRequest() {
    //EXCLUDE LINES AFTER >>>>
    std::vector<Server> filteredServers;
    for (std::vector<Server>::iterator itServer = _candidateServers.begin(); itServer != _candidateServers.end(); ++itServer) {
        std::vector<Listen> hostPort = itServer->get_listeners();
        for (std::vector<Listen>::iterator itHostPort = hostPort.begin(); itHostPort != hostPort.end(); ++itHostPort) {
            if (itHostPort->port == _parsedRequest.port) {
                filteredServers.push_back(*itServer);
            }
        }
    }
    _candidateServers = filteredServers;
    //EXCLUDE LINES BEFORE <<<<
	for (std::vector<Server>::iterator itServer = _candidateServers.begin(); itServer != _candidateServers.end(); ++itServer) {
		std::vector<std::string> servernames = itServer->get_server_name();
		for (std::vector<std::string>::iterator itServerName = servernames.begin(); itServerName != servernames.end(); ++itServerName) {
			if (*itServerName == _parsedRequest.host) {
				_server = *itServer;
				return;
			}
		}
	}
	_server = _candidateServers[0];
}

void ResponseBuilder::defineLocation() {
    std::vector<Location> locations = _server.get_location();
    for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); ++it) {
        if (_parsedRequest.path == it->get_path()) {
            _location = *it;
            std::string body ="Location found!\n\n" + _server.get_root() + _location.get_path();
            std::string contentLength = std::to_string(body.length());
            _response = "HTTP/1.1 200 OK\r\nContent-Length:" + contentLength + "\r\n\r\n" + body;
            return;
        }
    }
    throw NoLocationException();
}

bool ResponseBuilder::isMethodAllowed(const std::string &method) {

    if (std::find(_location.get_methods().begin(), _location.get_methods().end(), method) == _location.get_methods().end()) {
        return false;
    }
    return true;
}

bool ResponseBuilder::isBodySizeAllowed() {
    if (_parsedRequest.body.length() > _location.get_client_max_body_size()) {
        return false;
    }
    return true;
}

void ResponseBuilder::checkMethodAndBodySize() {
    std::string method ;
    if (_parsedRequest.method == GET) {
       method = "GET";
    }
    else if (_parsedRequest.method == POST) {
        method = "POST";
    }
    else if (_parsedRequest.method == DELETE) {
        method = "DELETE";
    }
    if (!isMethodAllowed(method)) {
        throw MethodNotAllowedException();
    }
    if (method == "POST" && !isBodySizeAllowed()) {
        throw BodySizeExceededException();
    }
}

bool ResponseBuilder::isFile() {
    //check if it is a file, if it is a file return the file
    //need to check how the file is searched in this step, using server root ?
    return false;
}

void ResponseBuilder::buildResponse() {

    try {
        delegateRequest();
        if (isFile())//check if it is a file, if it is a file return the file << NEXTP STEP
            return;
        defineLocation();
        checkMethodAndBodySize();
        searchLocation(); //check if there is an index file in the location, if not throw NoLocationException << STEP 2
    }
    catch (NoLocationException &e) {
        _response = ERROR_404; //create function to check if path exists; look for server index in directory path; checkif autoindex is on; define if it should be 404 or 403
        return;
    }
    catch (MethodNotAllowedException &e) {
        _response = ERROR_405; //create function to return right error page defineErrorPage(int error_code)
        return;
    }
    catch (BodySizeExceededException &e) {
        _response = ERROR_413; //create function to return right error page
        return;
    }
    catch (std::exception &e){
        _response = "HTTP/1.1 200 OK\r\nContent-Length: 39\r\n\r\nEXCEPTION NOT CAUGHT IN BUILDING RESPONSE!";
        Logger::log(LOG_WARNING, "No caught exception");
        Logger::log(LOG_WARNING, e.what());
    }

}

const std::string& ResponseBuilder::getResponse() const {
    return _response;
}