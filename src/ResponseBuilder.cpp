#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder() {
}

ResponseBuilder::~ResponseBuilder() {
}

void ResponseBuilder::setFd(int fd) {
    _fd = fd;
}

void ResponseBuilder::setCandidateServers(std::vector<Server> servers) {
    _candidateServers = servers;
}

void ResponseBuilder::setRequest(char *request) {
    _request = request;
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
    std::string method;
    if (_parsedRequest.method == GET) { //stop using Enum for GET, POST, DELETE, and only use std::string
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
    if (method == "GET" && !isBodySizeAllowed()) {
        throw BodySizeExceededException();
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
    if (path[0] == '/') //the presence or absence of the bar should not be ignored, see behavior described by Maragao
        path = path.substr(1);
    std::string file_path = server_root + path;
    if (isFile(file_path)) {
        _response.loadFromFile(file_path);
        return true;
    }

    std::string file_name;
    std::string index_file_path;
    std::vector<std::string> index = _server.get_index();
    for (size_t i = 0; i < index.size(); i++) {
        file_name = index[i];
        if (file_name[0] == '/')
            file_name = file_name.substr(1);
        if (file_name[file_name.size() - 1] == '/')
            file_name = file_name.substr(0, file_name.size() - 1);
        index_file_path = file_path + '/' +file_name;
        if (isFile(index_file_path)) {
            _response.loadFromFile(index_file_path);
            return true;
        }
    }
    return false;
}

void ResponseBuilder::loadResponseFromFile(std::string path) {
    std::ifstream fileStream(path.c_str());
    if (!fileStream) {
        throw ForbiddenException();
    }
    _response.loadFromFile(path);
}

void ResponseBuilder::searchAlias() {
    //search alias
    std::string alias = _location.get_alias();
    if (alias == "") {
        Logger::log(LOG_WARNING, "No alias found in location.");
        throw InternalServerErrorException();
    }
    std::string index_file_path = _location.search_index_file(alias);
    if (index_file_path == "") {
        throw ForbiddenException();
    }
    loadResponseFromFile(index_file_path);
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
            throw ForbiddenException();
        }
        file_path += "/" + index_file_path;
        loadResponseFromFile(index_file_path);
    }
}

void ResponseBuilder::searchLocation() {
    if (_location.get_root() == "") {
        searchAlias();
        return;
    }
    searchRoot();
}

void ResponseBuilder::buildResponse(int fd, std::vector<Server> servers, char *request) {

    try {
        Logger::log(LOG_INFO, "Request" + _parsedRequest.path + " received, building response");
        setFd(fd);
        setCandidateServers(servers);
        setRequest(request);
        _parsedRequest = RequestParser::parseRequest(_request);
        if (_parsedRequest.statusCode != 200) {
            _response.loadDefaultErrorPage(_parsedRequest.statusCode);
            return;
        }
        delegateRequest();
        if (pathIsFile()) //check if the path is a file using the server root and index
            return;
        defineLocation();
        checkMethodAndBodySize();
        searchLocation(); //check if there is an index file in the location, if not throw ForbiddenException
    }
    catch (ForbiddenException &e) {
        _response.loadDefaultErrorPage(403);
        return;
    }
    catch (NoLocationException &e) {
        _response.loadDefaultErrorPage(404); //create function to check if path exists; look for server index in directory path; checkif autoindex is on; define if it should be 404 or 403 << should not be done in searchLocation() ?
        return;
    }
    catch (MethodNotAllowedException &e) {
        _response.loadDefaultErrorPage(405);
        return;
    }
    catch (BodySizeExceededException &e) {
        _response.loadDefaultErrorPage(413);
        return;
    }
    catch (InternalServerErrorException &e) {
        _response.loadDefaultErrorPage(500);
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