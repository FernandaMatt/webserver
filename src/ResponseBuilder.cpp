#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder() {
}

ResponseBuilder::~ResponseBuilder() {
}

Response& ResponseBuilder::get_response() {return _response;}

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
    // file_name = index[i];
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
    _response.loadAutoIndex(path, _parsedRequest.path);
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
        // file_path += "/" + index_file_path;
        loadResponseFromFile(index_file_path);
        return;
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
    //error_page_path = _server.get_root() + error_page_path;
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
        else if (_parsedRequest.method == DELETE) {
            processDELETE();
        }
        else {
            throw MethodNotAllowedException();
        }
    }
    catch (BadRequestException &e) {
        defineErrorPage(400);
        return;
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

//METODO POST

std::string ResponseBuilder::vectorToString(const std::vector<char>& vec) {
    return std::string(vec.begin(), vec.end());
}

void ResponseBuilder::processPOST() {
    //if it is not, check that the location accepts POST
    //check if the content-type is multipart/form-data and decode the mesage
    //or if exist the header Transfer-Encoding: chunked and decode the message
    // if not, just create the file with the correct extension
    //save the content to a specific path
    defineLocation();
    checkMethodAndBodySize();

    std::map<std::string, std::vector<char>> map;
    if (isMultipartBody() && isChunkedBody()) {
        std::string file_content;
        map = postChunkedBody();
        file_content = vectorToString(map.begin()->second);
        map.clear();
        map = postMultipartBody(file_content);
        for (std::map<std::string, std::vector<char>>::iterator it = map.begin(); it != map.end(); it++) {
            writeToFile(it->first, it->second);
        }
    }
    else if (isMultipartBody()) {
        map = postMultipartBody(_parsedRequest.body);
        for (std::map<std::string, std::vector<char>>::iterator it = map.begin(); it != map.end(); it++) {
            writeToFile(it->first, it->second);
        }
    }
    else if (isChunkedBody()) {
        map = postChunkedBody();
        writeToFile(map.begin()->first, map.begin()->second);
    }
    else {
        map = postCompleteBody();
        writeToFile(map.begin()->first, map.begin()->second);
    }
    _response.setStatusMessage("HTTP/1.1 201 Created\r\n");
    std::string empty_str;
    empty_str.clear();
    _response.setResponseContent(empty_str);
    _response.setHttpHeaders(empty_str);
}

bool    ResponseBuilder::isChunkedBody() {
    if (_parsedRequest.headers.find("Transfer-Encoding") != _parsedRequest.headers.end()) {
        if (_parsedRequest.headers.find("Transfer-Encoding")->second == "chunked")
            return true;
    }
    return false;
}

bool    ResponseBuilder::isMultipartBody() {
    if (_parsedRequest.headers.find("Content-Type") != _parsedRequest.headers.end()) {
        std::string content_type = _parsedRequest.headers.find("Content-Type")->second;
        if (content_type.compare(0, 30, "multipart/form-data; boundary=") == 0)
                return true;
    }
    return false;
}

std::map<std::string, std::vector<char>>   ResponseBuilder::postChunkedBody() {
    std::vector<char> file_content;
    std::string content_type = getContentType();
    std::string filename = getFileName(content_type);
    std::string::size_type pos = 0;

    if (_parsedRequest.body.empty())
        throw BadRequestException(); //invalid format
    while (pos < _parsedRequest.body.size()) {
        std::string::size_type end = _parsedRequest.body.find("\r\n", pos);
        if (end == std::string::npos)
            throw BadRequestException(); //Formato invalido
        std::string size_str = _parsedRequest.body.substr(pos, end - pos);
        std::istringstream size_stream(size_str);
        std::size_t chunk_size;
        if (!(size_stream >> std::hex >> chunk_size))
            throw BadRequestException(); //Formato invalido
        pos = end + 2;
        if (chunk_size == 0)
            break;
        if (pos + chunk_size > _parsedRequest.body.size())
            throw BadRequestException(); //Formato invalido
        file_content.insert(file_content.end(), _parsedRequest.body.begin() + pos, _parsedRequest.body.begin() + pos + chunk_size);
        pos += chunk_size;
        if (pos + 2 > _parsedRequest.body.size() || _parsedRequest.body.substr(pos, 2) != "\r\n")
            throw BadRequestException();
        pos += 2;
    }
    std::map<std::string, std::vector<char>> map;
    map.insert(std::pair<std::string, std::vector<char>>(filename, file_content));
    return map;
}

std::map<std::string, std::vector<char>>   ResponseBuilder::postMultipartBody(std::string body_content) {
    if (_parsedRequest.body.empty())
        throw BadRequestException(); //invalid format
    std::map<std::string, std::vector<char>> map;
//find boundary
    std::string content_type = _parsedRequest.headers.find("Content-Type")->second;
    size_t pos = content_type.find('=');
    if (pos == std::string::npos)
        throw InternalServerErrorException();
    std::string boundary = content_type.substr(pos + 1);

    pos = 0;
    while (pos < body_content.size()) {
        //set headers
        size_t part_start = body_content.find(boundary, pos);
        if (part_start == std::string::npos)
            break;
        part_start += boundary.size() + 2;
        //find end of headers
        size_t headers_end = body_content.find("\r\n\r\n", part_start);
        if (headers_end == std::string::npos)
            break;
        std::string headers = body_content.substr(part_start, headers_end - part_start);
        part_start = headers_end + 4;
        std::string filename;
        std::string complete_filename;
        std::string content_type;
        //set part headers
        std::istringstream headers_stream(headers);
        std::string header;
        while (std::getline(headers_stream, header)) {
            if (header.find("Content-Disposition") != std::string::npos) {
                size_t filename_pos = header.find("filename=");
                if (filename_pos != std::string::npos) {
                    filename_pos += 10;
                    size_t filename_end = header.find("\"", filename_pos);
                    if (filename_end == std::string::npos)
                        throw BadRequestException(); //invalid format
                    filename = header.substr(filename_pos, filename_end - filename_pos);
                }
            } else if (header.find("Content-Type") != std::string::npos) {
                size_t content_type_pos = header.find(": ");
                if (content_type_pos == std::string::npos)
                    throw BadRequestException(); //invalid format
                content_type = header.substr(content_type_pos + 2);
            }
            //encontrar o final da parte
            size_t part_end = body_content.find(boundary, part_start);
            if (part_end == std::string::npos)
                part_end = body_content.size();
            else
                part_end -= 4;
            if (part_end < part_start)
                throw BadRequestException(); //invalid format
            //add o conteudo no vetor
            if (!filename.empty() && !content_type.empty()) {
                complete_filename = getFileName(filename, content_type);
                std::vector<char> file_content(body_content.begin() + part_start, body_content.begin() + part_end);
                map.insert(std::pair<std::string, std::vector<char>>(complete_filename, file_content));
                //writeToFile(complete_filename, file_content);
            }
            pos = part_end + 2;
        }
    }
    return map;
}

std::map<std::string, std::vector<char>>   ResponseBuilder::postCompleteBody() {
    if (_parsedRequest.body.empty())
        throw BadRequestException(); //invalid format
    std::map<std::string, std::vector<char>> map;
    std::vector<char> file_content;
    file_content.reserve(_parsedRequest.body.size());
    file_content.insert(file_content.end(), _parsedRequest.body.begin(), _parsedRequest.body.end());
    std::string content_type = getContentType();
    std::string filename = getFileName(content_type);
    map.insert(std::pair<std::string, std::vector<char>>(filename, file_content));
    return map;
}

std::string  ResponseBuilder::getContentType() {
    if (_parsedRequest.headers.find("Content-Type") != _parsedRequest.headers.end()) {
        std::string content_type = _parsedRequest.headers.find("Content-Type")->second;
        return content_type;
    }
    return NULL;
}

std::string  ResponseBuilder::getFileName(std::string& filename, std::string const& content_type) {
    MimeTypes mimes;
    std::string path;
    std::string complete_path;
    
    if (!_location.get_upload_path().empty() && _location.get_upload_path()[0] != '/') {
        if (!_location.get_root().empty())
            path = _location.get_root() + '/' + _location.get_upload_path() + '/';
        else
            path = _location.get_alias() + '/' + _location.get_upload_path() + '/';
    }
    else
        path = _location.get_upload_path() + '/';
    
    if (!isDirectory(path))
        throw ForbiddenException();
    
    if (!filename.empty()) {
        if (filename[0] == '/')
            filename = filename.substr(1);
        complete_path = path + filename;
        return generateUniqueFilename(complete_path);
    }

    size_t pos = content_type.find('/');
    if (pos == std::string::npos)
        filename = "file";
    else
        filename = content_type.substr(0, pos);
    complete_path = path + filename + mimes.getExtension(content_type);
    return generateUniqueFilename(complete_path);
}

std::string  ResponseBuilder::getFileName(std::string const& content_type) {
    MimeTypes mimes;
    std::string path;
    std::string complete_path;

    if (!_location.get_upload_path().empty() && _location.get_upload_path()[0] != '/') {
        if (!_location.get_root().empty())
            path = _location.get_root() + '/' + _location.get_upload_path() + '/';
        else
            path = _location.get_alias() + '/' + _location.get_upload_path() + '/';        
    }
    else
        path = _location.get_upload_path() + '/';

    if (!isDirectory(path))
        throw ForbiddenException();

    size_t pos = content_type.find('/');
    std::string filename;
    if (pos == std::string::npos)
        filename = "file";
    else
        filename = content_type.substr(0, pos);
    complete_path = path + filename + mimes.getExtension(content_type);
    return generateUniqueFilename(complete_path);
}

std::string ResponseBuilder::generateUniqueFilename(const std::string& file_path) {
    std::string complete_path = file_path;
    size_t pos_ext = complete_path.find_last_of('.');
    std::string base = complete_path.substr(0, pos_ext);
    std::string ext = (pos_ext == std::string::npos) ? "" : complete_path.substr(pos_ext);

    for (int i = 1; isFile(complete_path); i++) {
        complete_path = base + '(' + std::to_string(i) + ')' + ext;
    }
    return complete_path;
}

void    ResponseBuilder::writeToFile(std::string const filename, std::vector<char> const file_content) {
    if (file_content.empty())
        throw BadRequestException(); //invalid format
    std::ofstream outfile(filename.c_str(), std::ios::out | std::ios::binary);
    if (!outfile)
        throw InternalServerErrorException();
    outfile.write(&file_content[0], file_content.size());
    if (!outfile)
        throw InternalServerErrorException();
    outfile.close();
    if (!outfile.good())
        throw InternalServerErrorException();
}


//METODO DELETE


void ResponseBuilder::processDELETE() {
    
    std::string path;
    std::string filename;
    size_t pos = _parsedRequest.path.find_last_of('/');
    if (pos == std::string::npos)
        throw BadRequestException();
    
    if (pos == 0) {
        path = "/";
        if (_parsedRequest.path.size() > 1)
            filename = _parsedRequest.path.substr(1);
    }
    else {
        path = _parsedRequest.path.substr(0, pos);
        if (_parsedRequest.path.size() > pos + 1)
            filename = _parsedRequest.path.substr(pos + 1);
    }

    bool location_found = false;
    std::vector<Location> locations = _server.get_location();
    for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); it++) {
        if (it->get_path() == path) {
            _location = *it;
            location_found = true;
            break;
        }
    }
    if (location_found == false)
        throw BadRequestException();
    
    checkMethodAndBodySize();

    std::string complete_path;
    if (!filename.empty()) {
        if (!_location.get_root().empty())
            complete_path = _location.get_root() + _location.get_path() + '/' + filename;
        else
            complete_path = _location.get_alias() + '/' + filename;
    }
    else {
        if (!_location.get_root().empty())
            complete_path = _location.get_root() + _location.get_path();
        else
            complete_path = _location.get_alias();
    }

    if (isFile(complete_path) && access(complete_path.c_str(), F_OK)) {
       if (std::remove(complete_path.c_str()) != 0)
            throw InternalServerErrorException();
    }
    else
        throw NoLocationException();

    _response.setStatusMessage("HTTP/1.1 204 No Content\r\n");
    std::string empty_str;
    empty_str.clear();
    _response.setResponseContent(empty_str);
    _response.setHttpHeaders(empty_str);
}