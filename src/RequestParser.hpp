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
		~RequestParser();

		static httpRequest parseRequest(std::string request);

	private:
		RequestParser();
		RequestParser(const RequestParser &src);
		RequestParser &operator=(const RequestParser &src);
		static httpMethod							getMethod(std::string &parsing_request);
		static std::string  						getPath(std::string &parsing_request);
        static std::string                          getResourceType(std::string url);
		static std::string  						getCGIPath(std::string url);
		static std::string  						getExtraPath(std::string url);
        static std::string  						getCGIfilename(std::string url);
        static std::string                          getQueryString(std::string &parsing_request);
		static std::map<std::string, std::string>	getQueryVariables(std::string &parsing_request);
		static std::string  						getVersion(std::string &parsing_request);
		static std::map<std::string, std::string>	getHeaders(std::string &parsing_request);
		static std::string							getBody(std::string &parsing_request);
		static std::string							getHost(std::string hostPort);
		static std::string							getPort(std::string hostPort);
		static std::string							getRequestStatus(httpRequest &request);
		static httpMethod							stringToHttpMethod(const std::string& method);
		static std::string							httpMethodToString(const httpMethod &method);
		static bool    								isChunkedBody(httpRequest &request);
		static std::string							isChunkedComplete(std::string &body);
        static bool    								isMultipartBody(httpRequest &request);
		static std::string							isMultipartComplete(httpRequest &request);
};