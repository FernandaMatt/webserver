#pragma once

#include <string>
#include <map>
#include "../defines.hpp"
#include <iostream>
#include <stdexcept>
#include <exception>
#include <ostream>
#include <vector>

class RequestParser
{
	public:
		RequestParser();
		RequestParser(const RequestParser &src);
		~RequestParser();
		RequestParser &operator=(const RequestParser &src);

		httpRequest parseRequest(std::string request);

	private:
		httpMethod							getMethod(std::string &parsing_request);
		std::string 						getPath(std::string &parsing_request);
		std::string 						getVersion(std::string &parsing_request);
		std::map<std::string, std::string>	getHeaders(std::string &parsing_request);
		std::string							getBody(std::string &parsing_request);
		std::string							getHost(std::string hostPort);
		std::string							getPort(std::string hostPort);
		httpMethod							stringToHttpMethod(const std::string& method);
		std::string							httpMethodToString(const httpMethod &method);
};