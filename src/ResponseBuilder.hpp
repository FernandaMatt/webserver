#pragma once

#include <string>
#include <iostream>
#include "../Server.hpp"
#include "RequestParser.hpp"


class ResponseBuilder {
	public:
		ResponseBuilder();
		ResponseBuilder(int fd, std::vector<Server> servers, char *request);
		~ResponseBuilder();

		void printInitializedAttributes();
        void buildResponse();

	private:
		int _fd;
		std::vector<Server> _candidateServers;
		char *_request;
		httpRequest _parsedRequest;
		std::string _response;
		Server _server;

		void delegateRequest();
};