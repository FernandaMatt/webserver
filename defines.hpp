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
	std::string statusCode;

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

#define ERROR_404 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length:507 \r\n\r\n\
<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>404 Not Found</title>\
    <style>\
        body {\
            font-family: Arial, sans-serif;\
            text-align: center;\
            margin-top: 50px;\
        }\
        h1 {\
            font-size: 100px;\
        }\
        p {\
            font-size: 20px;\
        }\
    </style>\
</head>\
<body>\
    <h1>404</h1>\
    <p>Not Found</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

#define HEADER_403 "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n"
#define HEADER_405 "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n"
#define HEADER_500 "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n"

#endif
