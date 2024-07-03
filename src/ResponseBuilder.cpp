#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder() {
}

ResponseBuilder::~ResponseBuilder() {
}

void ResponseBuilder::printInitializedAttributes() {
	std::cout << GRN << "servers: " << RST << std::endl;
	std::cout << GRN << "parsedRequest: " << RST << std::endl;
	_parsedRequest.printRequest();
}

void ResponseBuilder::defineLocation() {
    std::vector<Location> locations = _server.get_location();
    for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); ++it) {
        if (_parsedRequest.path == it->get_path()) {
            _location = *it;
            return;
        }
    }
    std::string full_path = _server.get_root() + _parsedRequest.path; //check if path starts w/ / -> see behavior described by Maragao
    if (isDirectory(full_path)) {
        if (checkAutoIndex(full_path))
            return;
        throw ForbiddenException();
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

void ResponseBuilder::checkMethodAndBodySize() { //NEED TO MAKE MORE TESTES to check if the verification are correct
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

bool ResponseBuilder::isDirectory(std::string path) {
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

    // std::string file_name;
    // std::string index_file_path;
    // std::vector<std::string> index = _server.get_index();
    // for (size_t i = 0; i < index.size(); i++) {
    //     file_name = index[i];
    //     if (file_name[0] == '/')
    //         file_name = file_name.substr(1);
    //     if (file_name[file_name.size() - 1] == '/')
    //         file_name = file_name.substr(0, file_name.size() - 1);
    //     index_file_path = file_path + '/' +file_name;
    //     if (isFile(index_file_path)) {
    //         _response.loadFromFile(index_file_path);
    //         return true;
    //     }
    // }
    return false;
}

void ResponseBuilder::loadResponseFromFile(std::string path) {
    std::ifstream fileStream(path.c_str());
    if (!fileStream) {
        throw ForbiddenException();
    }
    _response.loadFromFile(path);
}

bool ResponseBuilder::checkAutoIndex(std::string &path) {
    if (_location.get_path() != "" && _location.get_autoindex() != "on"
        || _location.get_path() == "" && _server.get_autoindex() != "on")
        return false;
    _response.loadAutoIndex(path);
    return true;
}

void ResponseBuilder::searchAlias() {
    std::string alias = _location.get_alias();
    if (alias == "") {
        Logger::log(LOG_WARNING, "No alias found in location.");
        throw InternalServerErrorException();
    }
    std::string index_file_path = _location.search_index_file(alias);
    if (index_file_path == "") {
        if (checkAutoIndex(alias))
            return;
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
    if (isFile(file_path)) {
        loadResponseFromFile(file_path);
        return;
    }
    if (isDirectory(file_path)) {

        std::string index_file_path = _location.search_index_file(file_path);
        if (index_file_path == "") {
            if (checkAutoIndex(file_path))
                return;
            throw ForbiddenException();
        }
        file_path += "/" + index_file_path;
        loadResponseFromFile(index_file_path);
    }
    _response.loadDefaultErrorPage(403);// CHECK THE RIGHT ERROR when the path configured in the .conf file does not exist
}

void ResponseBuilder::searchLocation() {
    if (_location.get_root() == "") {
        searchAlias();
        return;
    }
    searchRoot();
}

void ResponseBuilder::defineErrorPage(int statusCode) {
    std::map<int, std::string> error_pages = _server.get_error_pages();
    std::string error_page_path = error_pages[statusCode];
    if (error_page_path == "") {
        Logger::log(LOG_INFO, "No error page found for error code " + std::to_string(statusCode) + ". Loading default error page.");
        _response.loadDefaultErrorPage(statusCode);
    }
    error_page_path = _server.get_root() + error_page_path;
    try {
        loadResponseFromFile(error_page_path);
    }
    catch (ForbiddenException &e) { //STOP USING EXCEPTIONS FOR FLOW CONTROL <<<< This catch is inside another catch, which is not good
        Logger::log(LOG_WARNING, "Custom Error page set, but file not found. Loading default error page.");
        _response.loadDefaultErrorPage(statusCode);
        return;
    }
}

bool ResponseBuilder::isCGI() {
    // std::string path = _parsedRequest.path;
    // std::string cgi_extension = _server.get_cgi_extension();
    // if (path.length() < cgi_extension.length())
    //     return false;
    // if (path.substr(path.length() - cgi_extension.length()) == cgi_extension)
    //     return true;
    return false;
}

void ResponseBuilder::processGET() {
    //check if the path is a cgi file, if it is, execute the cgi
    if (pathIsFile()) //check if the path is a file using the server root and index
        return;
    defineLocation();
    if (_response.loaded)
        return;
    checkMethodAndBodySize();
    searchLocation(); //check if there is an index file in the location, if not throw ForbiddenException
}

void ResponseBuilder::processPOST() {
    //check if the path ia a cgi script
    //if it is, execute the cgi
    //if it is not, check that the location accepts POST
    //save the content to a specific path
}

void ResponseBuilder::processDELETE() {
    //check if the path is a file
    //delete the file
}

void ResponseBuilder::buildResponse(Server server, httpRequest request) {

    try {
        Logger::log(LOG_INFO, "Request" + _parsedRequest.path + " received, building response");
        _server = server;
        _response.loaded= false;
        _location.set_path("");
        _parsedRequest = request;
        if (_parsedRequest.statusCode != 200) {
            _response.loadDefaultErrorPage(_parsedRequest.statusCode);
            return;
        }
        if (_parsedRequest.method == GET)
            processGET();
        else if (_parsedRequest.method == POST)
            processPOST();
        else if (_parsedRequest.method == DELETE)
            processDELETE();
        else {
            throw MethodNotAllowedException();
        }
    }
    catch (ForbiddenException &e) {
        defineErrorPage(403);
        return;
    }
    catch (NoLocationException &e) {
        defineErrorPage(404);
        return;
    }
    catch (MethodNotAllowedException &e) {
        defineErrorPage(405);
        return;
    }
    catch (BodySizeExceededException &e) {
        defineErrorPage(413);
        return;
    }
    catch (InternalServerErrorException &e) {
        defineErrorPage(500);
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
