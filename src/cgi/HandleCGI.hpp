#ifndef HANDLECGI_HPP
# define HANDLECGI_HPP

# include "../utils/defines.hpp"
# include "../config/Server.hpp"
# include "../config/Location.hpp"
# include "../http/ResponseBuilder.hpp"
# include <vector>
# include <string>
# include <iostream>
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/epoll.h>
# include <fcntl.h>
# include <string.h>
# include <cstdlib>
# include <cstdio>
# include <cstring>

class HandleCGI {
    private:
        httpRequest                 _request;
        std::vector<std::string>    _env;
        int                         _fdEpool;
        Location                    _location;

        bool        getCGILocation();
        bool        isMethodAllowed(const std::string &method);
        int         forbiddenMethodOrBodySize();
        bool        isBodySizeAllowed();
        std::string getFullCGIPath();
        std::string getCGIPath();
        std::string getPathTranslated();
        bool        isDirectory(std::string path);
        bool        isFile(std::string path);
        char        **buildEnv();
        char        **convertEnv();
        void        freeEnv(char **env);
        void        sendErrorResponse(int statusCode, int fd);
        std::map<std::string, std::string> parseCGIHeaders();
        std::string getCGIHeaders();
        std::string getCGIBody();
        std::string getMethod(int method);

    public:
        HandleCGI();
        ~HandleCGI();
        HandleCGI(httpRequest parsedRequest, int &fdEpool, int responseFd, Server server);

        int         executeCGI();
        Response    getCGIResponse();

        int         _pipefd[2];
        int         _responseFd;
        bool        responseReady;
        Server      _server;
        std::string _responseCGI;

};

#endif