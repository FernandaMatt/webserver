#include "Response.hpp"

Response::Response()
        : _statusMessage(stringToVector("HTTP/1.1 200 OK\r\n")),
          _httpHeaders(stringToVector("Content-Length: 39\r\n\r\n")),
          _responseContent(stringToVector("Default Response from Response Builder!")) {}

Response::~Response() {}

std::string Response::getContentType(const std::string& filePath) {
    MimeTypes mimeTypes;
    std::string::size_type idx = filePath.rfind('.');
    if (idx != std::string::npos) {
        return (mimeTypes.getType(filePath.substr(idx)));
    }
    return "application/octet-stream";
}

std::vector<char> Response::stringToVector(const std::string& str) {
    return std::vector<char>(str.begin(), str.end());
}

void Response::loadFromFile(const std::string& filePath, bool logError)
{
    _responseContent.clear();

    std::ifstream fileStream(filePath.c_str(), std::ios::binary);
    if (!fileStream) {
        if (logError)
            Logger::log(LOG_ERROR, "Error opening file: " + filePath);
        loadDefaultErrorPage(404);
        return;
    }

    fileStream.seekg(0, std::ios::end);
    std::streamsize size = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);

    if (!setResponseContent(fileStream, size, filePath))
        return;

    std::string contentType = getContentType(filePath);
    std::ostringstream oss;
    oss << "Content-Length: " << _responseContent.size() << "\r\n";
    oss << "Content-Type: " << contentType << "\r\n\r\n";
    setHttpHeaders(oss.str());
    loaded = true;
}

std::vector<FileInfo> Response::getFilesInDirectory(const std::string &directoryPath) {
    std::vector<FileInfo> filesInfo;
    DIR *dir;
    struct dirent *ent;
    struct stat fileStat;
    if ((dir = opendir(directoryPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string filePath = directoryPath + "/" + std::string(ent->d_name);
            if (stat(filePath.c_str(), &fileStat) == 0) {
                FileInfo fileInfo;
                fileInfo.name = ent->d_name;
                fileInfo.lastModTime = fileStat.st_mtime;
                fileInfo.size = fileStat.st_size;
                filesInfo.push_back(fileInfo);
            }
        }
        closedir(dir);
    } else {
        Logger::log(LOG_ERROR, "Error opening directory: " + directoryPath);
    }

    std::vector<FileInfo>::iterator dotIt = std::find_if(filesInfo.begin(), filesInfo.end(), [](const FileInfo& fileInfo) {
        return fileInfo.name == ".";
    });
    if (dotIt != filesInfo.end()) {
        char *dirPathMutable = strdup(directoryPath.c_str()); // Duplicate directoryPath since basename might modify its input
        char *folderName = basename(dirPathMutable); // Get the actual folder name
        dotIt->name = folderName;
        free(dirPathMutable);
        std::iter_swap(filesInfo.begin(), dotIt);
    }

    std::vector<FileInfo>::iterator dotDotIt = std::find_if(filesInfo.begin() + 1, filesInfo.end(), [](const FileInfo& fileInfo) {
        return fileInfo.name == "..";
    });
    if (dotDotIt != filesInfo.end()) {
        std::iter_swap(filesInfo.begin() + 1, dotDotIt);
    }

    std::sort(filesInfo.begin() + 2, filesInfo.end(), [](const FileInfo& a, const FileInfo& b) {
        return a.name < b.name;
    });

    return filesInfo;
}

void Response::loadAutoIndex(std::string &path, std::string &request_path) {
    std::vector<FileInfo> filesInfo = getFilesInDirectory(path);
    std::string currentDir;
    if (request_path.find("/") == 0)
        currentDir = request_path + "/";
    else
        currentDir = "/" + request_path + "/";

    std::string autoIndex = "<html><head><title>Index</title>";
    autoIndex += "<style>table { border-collapse: collapse; } th, td { padding: 10px; }</style>";
    autoIndex += "</head><body><h1>Index of " + currentDir + "</h1>";

    autoIndex += "<table><tr><th>Name</th><th>Last Modified Time</th><th>Size</th></tr>";
    for (size_t i = 1; i < filesInfo.size(); i++) {
        autoIndex += "<tr>";
        autoIndex += "<td><a href=\"" + currentDir + filesInfo[i].name + "\">" + filesInfo[i].name + "</a></td>";

        struct tm *timeinfo = localtime(&filesInfo[i].lastModTime);
        char timeBuffer[80];
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        autoIndex += "<td>" + std::string(timeBuffer) + "</td>";

        std::ostringstream sizeStream;
        sizeStream << filesInfo[i].size;
        autoIndex += "<td>" + sizeStream.str() + "</td>";
        autoIndex += "</tr>";
    }
    autoIndex += "</table></body></html>";
    setStatusMessage("HTTP/1.1 200 OK\r\n");
    std::ostringstream oss;
    oss << "Content-Length: " << autoIndex.size() << "\r\n";
    oss << "Content-Type: text/html\r\n\r\n";
    setHttpHeaders(oss.str());
    setResponseContent(autoIndex);
    loaded = true;
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

void Response::setStatusMessage(const std::string statusMessage) {
    _statusMessage.clear();
    _statusMessage = stringToVector(statusMessage);
}

void Response::setHttpHeaders(const std::string httpHeaders) {
    _httpHeaders.clear();
    _httpHeaders = stringToVector(httpHeaders);
}

void Response::setResponseContent(const std::string responseContent) {
    _responseContent.clear();
    _responseContent = stringToVector(responseContent);
}

bool Response::setResponseContent(std::ifstream &ifs, std::streamsize &size, const std::string &filePath) {
    _responseContent.clear();
    _responseContent.resize(size);
    if (!ifs.read(_responseContent.data(), size)) {
        Logger::log(LOG_ERROR, "Error reading file: " + filePath);
        loadDefaultErrorPage(500); //use throw
        return false;
    }
    return true;
}

void Response::loadErrorPage(int statusCode, Server server, bool logError) {

    _statusMessage.clear();
    _httpHeaders.clear();
    _responseContent.clear();

    std::map<int, std::string> error_pages = server.get_error_pages();

    if (error_pages.find(statusCode) == error_pages.end()) {
        if (logError)
            Logger::log(LOG_INFO, "No error page found for error code " + std::to_string(statusCode) + ". Loading default error page.");
        loadDefaultErrorPage(statusCode);
        return;
    }

    std::string error_page_path = error_pages[statusCode];

    if (error_page_path == "") {
        if (logError)
            Logger::log(LOG_INFO, "No error page found for error code " + std::to_string(400) + ". Loading default error page.");
        loadDefaultErrorPage(statusCode);
        return;
    }

    std::string serverRoot = server.get_root();
    std::string error_page_full_path;
    if (error_page_path[0] == '/')
        error_page_path = error_page_path.substr(1);
    if (serverRoot[serverRoot.size() - 1] == '/') {
        error_page_full_path = serverRoot + error_page_path;
    } else {
        error_page_full_path = serverRoot + "/" + error_page_path;
    }

    std::ifstream fileStream(error_page_full_path.c_str());
    if (!fileStream) {
        if (logError)
            Logger::log(LOG_WARNING, "Custom Error page set, but file not found. Loading default error page.");
        loadDefaultErrorPage(statusCode);
        return;
    }

    loadFromFile(error_page_full_path);
}

void Response::setDefaultErrorPage(int statusCode, const std::string &title) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " " << title << "\r\n";
    std::string statusMessage = oss.str();
    std::cout << "statusMessage = " << statusMessage << std::endl;

    oss.str("");
    oss << "<!DOCTYPE html>";
    oss << "<html lang='en'>";
    oss << "<head>";
    oss << "<meta charset='UTF-8'>";
    oss << "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    oss << "<title>" << statusCode << " " << title <<"</title>";
    oss << "<style>";
    oss << "body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }";
    oss << "h1 { font-size: 100px; }";
    oss << "p { font-size: 20px; }";
    oss << "</style>";
    oss << "</head>";
    oss << "<body>";
    oss << "<h1>" << statusCode << "</h1>";
    oss << "<p>" << title << "</p>";
    oss << "<p>by Fe, Iza e Mari</p>";
    oss << "</body>";
    oss << "</html>";

    std::string responseContent = oss.str();

    oss.str("");

    oss << "Content-Type: text/html\r\nContent-Length:" << responseContent.size() << " \r\n\r\n";
    std::string header = oss.str();

    setStatusMessage(statusMessage);
    setHttpHeaders(header);
    setResponseContent(responseContent);
}

void Response::setErrorStatusMessage(int statusCode) {
    switch (statusCode) {
        case 400:
            setStatusMessage("HTTP/1.1 400 Bad Request\r\n");
            break;
        case 401:
            setStatusMessage("HTTP/1.1 401 Unauthorized\r\n");
            break;
        case 403:
            setStatusMessage("HTTP/1.1 403 Forbidden\r\n");
            break;
        case 404:
            setStatusMessage("HTTP/1.1 404 Not Found\r\n");
            break;
        case 405:
            setStatusMessage("HTTP/1.1 405 Method Not Allowed\r\n");
            break;
        case 413:
            setStatusMessage("HTTP/1.1 413 Request Entity Too Large\r\n");
            break;
        case 500:
            setStatusMessage("HTTP/1.1 500 Internal Server Error\r\n");
            break;
        case 501:
            setStatusMessage("HTTP/1.1 501 Not Implemented\r\n");
            break;
        case 502:
            setStatusMessage("HTTP/1.1 502 Bad Gateway\r\n");
            break;
        case 503:
            setStatusMessage("HTTP/1.1 503 Service Unavailable\r\n");
            break;
        default:
            setStatusMessage("HTTP/1.1 500 Internal Server Error\r\n");
            break;
    }

}

void Response::loadDefaultErrorPage(int statusCode) {
    switch (statusCode) {
        case 400:
            setDefaultErrorPage(400, "Bad Request");
            break;
        case 401:
            setDefaultErrorPage(401, "Unauthorized");
            break;
        case 403:
            setDefaultErrorPage(403, "Forbidden");
            break;
        case 404:
            setDefaultErrorPage(404, "Not Found");
            break;
        case 405:
            setDefaultErrorPage(405, "Method Not Allowed");
            break;
        case 413:
            setDefaultErrorPage(413, "Request Entity Too Large");
            break;
        case 500:
            setDefaultErrorPage(500, "Internal Server Error");
            break;
        case 501:
            setDefaultErrorPage(501, "Not Implemented");
            break;
        case 502:
            setDefaultErrorPage(502, "Bad Gateway");
            break;
        case 503:
            setDefaultErrorPage(503, "Service Unavailable");
            break;
        default:
            setDefaultErrorPage(500, "Internal Server Error");
            break;
    }
}

int Response::getResponseSize() const {
    return _statusMessage.size() + _httpHeaders.size() + _responseContent.size();
}

const std::vector<char> Response::getResponse() const {
    std::vector<char> response;
    response.insert(response.end(), _statusMessage.begin(), _statusMessage.end());
    response.insert(response.end(), _httpHeaders.begin(), _httpHeaders.end());
    response.insert(response.end(), _responseContent.begin(), _responseContent.end());
    return response;
}
