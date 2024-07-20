#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <vector>
# include <map>
# include <string>
# include <fstream>
# include <sstream>
# include <dirent.h>
# include <sys/stat.h>
# include <algorithm>
# include <libgen.h>
# include <string.h>
# include "../utils/Logger.hpp"
# include "../utils/error_pages.hpp"
# include "../utils/MimeTypes.hpp"
# include "../config/Server.hpp"

struct FileInfo {
    std::string name;
    time_t lastModTime;
    off_t size;
};

class Response {

    public:
        Response();
        ~Response();

        void loadFromFile(const std::string& filePath, bool logError = true);
        void loadAutoIndex(std::string &path, std::string &request_path);
        void loadErrorPage(int statusCode, Server server, bool logError = true);
        void loadDefaultErrorPage(int statusCode);
        void setDefaultErrorPage(int statusCode, const std::string &message);
        const std::vector<char> getResponse() const;
        int getResponseSize() const;

        void setStatusMessage(const char *statusMessage); //see if i can receive the macro as string
        void setHttpHeaders(const char *httpHeaders);
        void setResponseContent(const char *responseContent);
        void setStatusMessage(const std::string statusMessage); //see if i can receive the macro as string
        void setHttpHeaders(const std::string httpHeaders);
        void setResponseContent(const std::string responseContent);
        bool setResponseContent(std::ifstream &ifs, std::streamsize &size, const std::string &filePath);

        bool loaded = false;

        friend std::ostream& operator<<(std::ostream& os, const Response& response) {
            os.write(response._statusMessage.data(), response._statusMessage.size());
            os.write(response._httpHeaders.data(), response._httpHeaders.size());
            os.write(response._responseContent.data(), response._responseContent.size());
            return os;
        }

    private:
        std::vector<char>       _statusMessage;
        std::vector<char>       _httpHeaders;
        std::vector<char>       _responseContent;
        std::string             getContentType(const std::string& filePath);
        std::vector<char>       stringToVector(const std::string& str);
        std::vector<FileInfo>   getFilesInDirectory(const std::string &directoryPath);
};

#endif
