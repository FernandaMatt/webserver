#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder() {
}

ResponseBuilder::ResponseBuilder(int fd, std::vector<Server> servers, char *request) : 
	_fd(fd), _candidateServers(servers), _request(request) {
	RequestParser parser;
	_parsedRequest = parser.parseRequest(request);
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
            return;
        }
    }
    throw NoLocationException();
}

bool ResponseBuilder::isMethodAllowed() {
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
    if (std::find(_server.get_methods().begin(), _server.get_methods().end(), method) == _server.get_methods().end()) {
        return false;
    }
    return true;
}

void ResponseBuilder::buildResponse() {

    try {
        std::cout << ERROR_404 << std::endl;
        delegateRequest();
        defineLocation();
        if (!isMethodAllowed())
            throw MethodNotAllowedException();
    }
    catch (NoLocationException &e) {
        _response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n"; //create function to check if path exists; look for server index in directory path; checkif autoindex is on; define if it should be 404 or 403
        return;
    }
    catch (MethodNotAllowedException &e) {
        _response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n"; //create function to return right error page
        return;
    }
    catch (std::exception &e){
        Logger::log(LOG_WARNING, "No caught exception");
        Logger::log(LOG_WARNING, e.what());
    }

}