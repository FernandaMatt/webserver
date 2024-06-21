#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder() {
}

ResponseBuilder::ResponseBuilder(int fd, std::vector<Server> servers, char *request) : 
	_fd(fd), _candidateServers(servers), _request(request) {
	RequestParser parser;
	_parsedRequest = parser.parseRequest(request); //turn into static function and in the buildResponse function
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
            // std::string body ="Location found!\n\n" + _server.get_root() + _location.get_path();
            // std::string contentLength = std::to_string(body.length());
            // _response.setStatusMessage("HTTP/1.1 200 OK\r\n");
            // std::string headers = "Content-Length:" + contentLength + "\r\n\r\n";
            // _response.setHttpHeaders(headers.c_str());
            // _response.setResponseContent(body.c_str());
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

bool isDirectory(std::string path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0) {
        return false;
    }
    return S_ISDIR(pathStat.st_mode);
}

bool ResponseBuilder::isFile(std::string path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0) {
        return false;
    }
    return S_ISREG(pathStat.st_mode);
}

bool ResponseBuilder::pathIsFile() {
    std::string server_root;
    std::string path;
    server_root = _server.get_root();
    if (server_root[server_root.length() - 1] != '/')
        server_root += "/";
    path = _parsedRequest.path;
    if (path[0] == '/')
        path = path.substr(1);
    std::string file_path = server_root + path;
    if (isFile(file_path)) {
        _response.loadFromFile(file_path);
        return true;
    }
    //need to check how the file is searched in this step, using server root ?
    return false;
}

void ResponseBuilder::loadResponseFromFile(std::string path) {
    std::ifstream fileStream(path.c_str());
    if (!fileStream) {
        throw NoLocationException(); //check if there is a root and index, but no file in the server folders the error is still 404
    }
    _response.loadFromFile(path);
}

void ResponseBuilder::searchRoot() {
    std::string location_root;
    std::string path;
    location_root = _location.get_root();
    if (location_root[location_root.length() - 1] != '/')
        location_root += "/";
    path = _parsedRequest.path;
    if (path[0] == '/')
        path = path.substr(1);
    std::string file_path = location_root + path;
    if (isFile(file_path)) { //check if the path comes with a file indicated -> !! Need to confirm if the file is returned or the error 403 (access not allowed)
        loadResponseFromFile(file_path);
        return;
    }
    if (isDirectory(file_path)) {
        
        std::string index_file_path = _location.search_index_file(file_path);
        if (index_file_path == "") {
            throw NoLocationException();
        }
        file_path += "/" + index_file_path;
        loadResponseFromFile(index_file_path);
    }
}

void ResponseBuilder::searchLocation() {
    if (_location.get_root() == "") {
        // searchAlias();
        return;
    }
    searchRoot();
}

void ResponseBuilder::buildResponse() {

    try {
        Logger::log(LOG_INFO, "Request" + _parsedRequest.path + " received, building response");
        delegateRequest();
        if (pathIsFile())//check if it is a file, if it is a file return the file it checked in the root of the server first
            return;
        defineLocation(); //make it insensible to '/' at the end of the path
        checkMethodAndBodySize();
        searchLocation(); //check if there is an index file in the location, if not throw NoLocationException << DOING
    }
    catch (NoLocationException &e) {
        _response.loadDefaultErrorPage(404); //create function to check if path exists; look for server index in directory path; checkif autoindex is on; define if it should be 404 or 403
        return;
    }
    catch (MethodNotAllowedException &e) {
        _response.loadDefaultErrorPage(405); //create function to return right error page defineErrorPage(int error_code)
        return;
    }
    catch (BodySizeExceededException &e) {
        _response.loadDefaultErrorPage(413); //create function to return right error page
        return;
    }
    catch (std::exception &e){
        _response.setStatusMessage("HTTP/1.1 200 OK\r\n");
        _response.setHttpHeaders("Content-Length: 39\r\n\r\n");
        _response.setResponseContent("EXCEPTION NOT CAUGHT IN BUILDING RESPONSE!");
        Logger::log(LOG_WARNING, "No caught exception");
        Logger::log(LOG_WARNING, e.what());
    }

}

const std::vector<char> ResponseBuilder::getResponse() const {
    return _response.getResponse();
}