#include "Response.hpp"

Response::Response()
        : _statusMessage(stringToVector("HTTP/1.1 200 OK\r\n")),
          _httpHeaders(stringToVector("Content-Length: 39\r\n\r\n")),
          _responseContent(stringToVector("Default Response from Response Builder!")) {}

Response::~Response() {}

std::string Response::getContentType(const std::string& filePath) {
    std::map<std::string, std::string> contentTypes = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".pdf", "application/pdf"}
    };
    std::string::size_type idx = filePath.rfind('.');
    if (idx != std::string::npos) {
        std::string extension = filePath.substr(idx);
        if (contentTypes.find(extension) != contentTypes.end()) {
            return contentTypes[extension];
        }
    }
    return "application/octet-stream";
}

std::vector<char> Response::stringToVector(const std::string& str) {
    return std::vector<char>(str.begin(), str.end());
}

void Response::loadFromFile(const std::string& filePath)
{
    std::ifstream fileStream(filePath.c_str(), std::ios::binary);
    if (!fileStream) {
        Logger::log(LOG_ERROR, "Error opening file: " + filePath);
        _statusMessage = stringToVector("HTTP/1.1 404 Not Found\r\n"); //create function do load default error page
        _httpHeaders = stringToVector("Content-Length: 0\r\n\r\n");
        _responseContent.clear();
        return;
    }

    fileStream.seekg(0, std::ios::end);
    std::streamsize size = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);

    _responseContent.resize(size);

    if (!fileStream.read(_responseContent.data(), size)) {
        Logger::log(LOG_ERROR, "Error reading file: " + filePath);
        _statusMessage = stringToVector("HTTP/1.1 500 Internal Server Error\r\n"); //create function do load default error page
        _httpHeaders = stringToVector("Content-Length: 0\r\n\r\n");
        _responseContent.clear();
        return;
    }

    std::string contentType = getContentType(filePath);
    std::ostringstream oss;
    oss << "Content-Length: " << _responseContent.size() << "\r\n";
    oss << "Content-Type: " << contentType << "\r\n\r\n";
    _httpHeaders = stringToVector(oss.str());
    _statusMessage = stringToVector("HTTP/1.1 200 OK\r\n");
}

void Response::setStatusMessage(const char *statusMessage) {
    _statusMessage.clear();
    _statusMessage = stringToVector(std::string(statusMessage));
}

void Response::setHttpHeaders(const char *httpHeaders) {
    _httpHeaders.clear();
    _httpHeaders = stringToVector(std::string(httpHeaders));
}

void Response::setResponseContent(const char *responseContent) {
    _responseContent.clear();
    _responseContent = stringToVector(std::string(responseContent));
}

void Response::loadDefaultErrorPage(int statusCode) {
    switch (statusCode) {
        case 404:
            setStatusMessage(STATUS_404);
            setHttpHeaders(HEADER_404);
            setResponseContent(HTML_404);
            break;
        case 405:
            setStatusMessage(STATUS_405);
            setHttpHeaders(HEADER_405);
            setResponseContent(HTML_405);
            break;
        case 413:
            setStatusMessage(STATUS_413);
            setHttpHeaders(HEADER_413);
            setResponseContent(HTML_413);
            break;
        default:
            setStatusMessage(STATUS_500);
            setHttpHeaders(HEADER_500);
            setResponseContent(HTML_500);
            break;
    }
}

const std::vector<char> Response::getResponse() const {
    std::vector<char> response;
    response.insert(response.end(), _statusMessage.begin(), _statusMessage.end());
    response.insert(response.end(), _httpHeaders.begin(), _httpHeaders.end());
    response.insert(response.end(), _responseContent.begin(), _responseContent.end());
    return response;
}