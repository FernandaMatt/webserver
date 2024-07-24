#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include "../config/Server.hpp"
#include "../utils/Logger.hpp"
#include "RequestParser.hpp"
#include "Response.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdio>
#include <map>


class ResponseBuilder {
	public:
		ResponseBuilder();
		~ResponseBuilder();

		void printInitializedAttributes();
        void buildResponse(Server server, httpRequest request);
        Response& get_response();

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

        class BadRequestException : public std::exception {
            public:
               virtual const char* what() const throw() {
                    return "Bad Request";
                }
        };

	private:
		httpRequest _parsedRequest;
		Response _response;
		Server _server;
        Location _location;

        void processGET();
        void processPOST();
        void processDELETE();
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

//post
        std::string vectorToString(const std::vector<char>& vec);
        bool    isChunkedBody();
        bool    isMultipartBody();
        std::map<std::string, std::vector<char>>   postChunkedBody();
        std::map<std::string, std::vector<char>>   postMultipartBody(std::string body_content);
        std::map<std::string, std::vector<char>>   postCompleteBody();
        std::string getContentType();
        std::string getFileName(std::string& filename, std::string const& content_type);
        std::string getFileName(std::string const& content_type);
        std::string generateUniqueFilename(const std::string& filepath);
        void    writeToFile(std::string const filename, std::vector<char> const decoded_body);

};
