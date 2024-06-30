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

void Response::loadFromFile(const std::string& filePath)
{
    _responseContent.clear();

    std::ifstream fileStream(filePath.c_str(), std::ios::binary);
    if (!fileStream) {
        Logger::log(LOG_ERROR, "Error opening file: " + filePath);
        loadDefaultErrorPage(404); //use throw
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
    setStatusMessage("HTTP/1.1 200 OK\r\n");
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

    auto dotIt = std::find_if(filesInfo.begin(), filesInfo.end(), [](const FileInfo& fileInfo) {
        return fileInfo.name == ".";
    });
    if (dotIt != filesInfo.end()) {
        char *dirPathMutable = strdup(directoryPath.c_str()); // Duplicate directoryPath since basename might modify its input
        char *folderName = basename(dirPathMutable); // Get the actual folder name
        dotIt->name = folderName;
        free(dirPathMutable);
        std::iter_swap(filesInfo.begin(), dotIt);
    }

    auto dotDotIt = std::find_if(filesInfo.begin() + 1, filesInfo.end(), [](const FileInfo& fileInfo) {
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

void Response::loadAutoIndex(std::string &path) {
    std::vector<FileInfo> filesInfo = getFilesInDirectory(path);
    std::string currentDir = "/" + filesInfo[0].name + "/";
    
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

void Response::loadDefaultErrorPage(int statusCode) {
    _responseContent.clear();
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
        case 403:
            setStatusMessage(STATUS_403);
            setHttpHeaders(HEADER_403);
            setResponseContent(HTML_403);
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