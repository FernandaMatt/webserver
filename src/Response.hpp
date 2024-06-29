#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "../Logger.hpp"
#include "error_pages.hpp"

class Response {

    public:
        Response();
        ~Response();

        void loadFromFile(const std::string& filePath);
        void loadDefaultErrorPage(int statusCode);
        const std::vector<char> getResponse() const;

        void setStatusMessage(const char *statusMessage); //see if i can receive the macro as string
        void setHttpHeaders(const char *httpHeaders);
        void setResponseContent(const char *responseContent);

        friend std::ostream& operator<<(std::ostream& os, const Response& response) {
            os.write(response._statusMessage.data(), response._statusMessage.size());
            os.write(response._httpHeaders.data(), response._httpHeaders.size());
            os.write(response._responseContent.data(), response._responseContent.size());
            return os;
        }

    private:
        std::vector<char>   _statusMessage;
        std::vector<char>   _httpHeaders;
        std::vector<char>   _responseContent;
        std::string         getContentType(const std::string& filePath);
        std::vector<char>   stringToVector(const std::string& str);
};
