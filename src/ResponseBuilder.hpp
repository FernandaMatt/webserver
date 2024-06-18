#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include "../Server.hpp"
#include "RequestParser.hpp"
#include "../Logger.hpp"


class ResponseBuilder {
	public:
		ResponseBuilder();
		ResponseBuilder(int fd, std::vector<Server> servers, char *request);
		~ResponseBuilder();

		void printInitializedAttributes();
        void buildResponse();

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

	private:
		int _fd;
		std::vector<Server> _candidateServers;
		char *_request;
		httpRequest _parsedRequest;
		std::string _response;
		Server _server;
        Location _location;

		void delegateRequest();
        bool isMethodAllowed();
        void defineLocation();
};