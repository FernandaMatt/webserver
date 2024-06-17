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
    //EXCLUDE LINES AFTER
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
    //EXCLUDE LINES BEFORE
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

void ResponseBuilder::buildResponse() {
    delegateRequest();
    std::cout << "Server: " << _server.get_root() << std::endl;
}