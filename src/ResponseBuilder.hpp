#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include "../Server.hpp"
#include "RequestParser.hpp"
#include "../Logger.hpp"
#include <sys/stat.h> 
#include <fstream>
#include <sstream>
#include "Response.hpp"

class ResponseBuilder {
	public:
		ResponseBuilder();
		~ResponseBuilder();

		void printInitializedAttributes();
        void buildResponse(int fd, std::vector<Server> servers, char *request);

        const std::vector<char> getResponse() const;

        class NoLocationException : public std::exception {
            public:
               virtual const char* what() const throw() {
                    return "No location found for the request";
                }
        };

        class MethodNotAllowedException : public std::exception {
            public:
               virtual const char* what() const throw() {
                    return "Method not allowed";
                }
        };

        class BodySizeExceededException : public std::exception {
            public:
               virtual const char* what() const throw() {
                    return "Body size exceeded";
                }
        };

        class InternalServerErrorException : public std::exception {
            public:
               virtual const char* what() const throw() {
                    return "Internal server error";
                }
        };

        class ForbiddenException : public std::exception {
            public:
               virtual const char* what() const throw() {
                    return "Forbidden";
                }
        };

	private:
		int _fd;
		std::vector<Server> _candidateServers;
		char *_request;
		httpRequest _parsedRequest;
		Response _response;
		Server _server;
        Location _location;

        void setFd(int fd);
        void setCandidateServers(std::vector<Server> servers);
        void setRequest(char *request);
    	void delegateRequest();
        bool isFile(std::string path);
        bool isDirectory(std::string path);
        bool pathIsFile();
        void defineLocation();
        void checkMethodAndBodySize();
        void searchLocation();
        bool isMethodAllowed(const std::string &method);
        bool isBodySizeAllowed();
        void loadResponseFromFile(std::string path); // Rewrite to use std::vector<char> and than load Response from any kind of file
        void defineErrorPage(int error_code);
        void searchRoot();
        void searchAlias();
        bool checkAutoIndex(std::string &path);
};