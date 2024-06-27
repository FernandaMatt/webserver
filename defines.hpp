#ifndef DEFINES_HPP
# define DEFINES_HPP

// OUTPUT COLORS

# define BLUE	"\033[34m"
# define RED		"\033[31m"
# define YLW		"\033[33m"
# define ORA		"\033[38;2;255;153;0m"
# define GRN		"\033[32m"
# define RST		"\033[0m"

// ENUMS

enum httpMethod
{
	GET,
	POST,
	DELETE
};

// STRUCTS

# include <iostream>
# include <map>
# include <string>
# include <ostream>

typedef struct httpRequest
{
	httpMethod method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string host;
	std::string port;
	std::string body;
	int statusCode;

	void printRequest()
	{
		std::cout << BLUE << "Method: " << RST << method << std::endl;
		std::cout << BLUE << "Path: " << RST << path << std::endl;
		std::cout << BLUE << "Version: " << RST << version << std::endl;
		std::cout << BLUE << "Headers: " << RST << std::endl;
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
			std::cout << "\t" << it->first << ": " << it->second << std::endl;
		std::cout << BLUE << "Host: " << RST << host << std::endl;
		std::cout << BLUE << "Port: " << RST << port << std::endl;
		std::cout << BLUE << "Body: " << RST << body << std::endl;
		std::cout << BLUE << "Status Code: " << RST << statusCode << std::endl;
	}
}				httpRequest;

#define MAX_EVENTS 10
#define BUF_SIZE 1024

#define LOGGING_ENABLED 1
#define	DEBUG_ENABLED 1

#endif
