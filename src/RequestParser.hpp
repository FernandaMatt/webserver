#pragma once

#include <string>
#include <map>
#include "../defines.hpp"
#include <iostream>
#include <stdexcept>
#include <exception>
#include <ostream>

class RequestParser
{
	public:
		RequestParser();
		RequestParser(const RequestParser &src);
		~RequestParser();
		RequestParser &operator=(const RequestParser &src);

		httpRequest parseRequest(std::string request);

	private:
		httpMethod							getMethod(std::string request);
		std::string 						getPath(std::string request);
		std::string 						getVersion(std::string request);
		std::map<std::string, std::string>	getHeaders(std::string request);
		std::string							getBody(std::string request);
		httpMethod							stringToHttpMethod(const std::string& method);
		std::string							httpMethodToString(const httpMethod &method);
};