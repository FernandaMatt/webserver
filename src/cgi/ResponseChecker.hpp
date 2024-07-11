#pragma once

#include <string>

class ResponseChecker
{
	public:
		ResponseChecker();
		ResponseChecker(std::string response);
		~ResponseChecker();

		int getResponseStatusCode();

	private:
        int         _statusCode;
        std::string _response;

		bool isStatusLineValid();
};