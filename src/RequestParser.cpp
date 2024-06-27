#include "RequestParser.hpp"

RequestParser::RequestParser()
{
}

RequestParser::RequestParser(const RequestParser &src)
{
	(void)src;
}

RequestParser::~RequestParser()
{
}

RequestParser &RequestParser::operator=(const RequestParser &src)
{
	(void)src;
	return *this;
}

httpRequest RequestParser::parseRequest(std::string request)
{
	std::string parsing_request;
	httpRequest req;

	parsing_request = request;
	try {
		req.method = getMethod(parsing_request);
		req.path = getPath(parsing_request);
		req.version = getVersion(parsing_request);
		req.headers = getHeaders(parsing_request);
		req.body = getBody(parsing_request);
		req.host = getHost(req.headers["Host"]);
		req.port = getPort(req.headers["Host"]);
	} catch (std::exception &e) {
		if (std::string(e.what()) == "Bad request")
			req.statusCode = 400;
		if (std::string(e.what()) == "Method not allowed")
			req.statusCode = 405;
		return req;
	}
	req.statusCode = 200;
	return req;
}

httpMethod RequestParser::getMethod(std::string &parsing_request)
{
	std::string method;

	if(parsing_request.empty())
		throw std::invalid_argument("Bad request");
	size_t pos = parsing_request.find(" ");
	if (pos == std::string::npos)
		throw std::invalid_argument("Bad request");
	method = parsing_request.substr(0, pos);
	if (method != "GET" && method != "POST" && method != "DELETE")
		throw std::invalid_argument("Method not allowed");
	parsing_request = parsing_request.substr(pos + 1);
	return (stringToHttpMethod(method));
}

std::string RequestParser::getPath(std::string &parsing_request)
{
	std::string url;

	if(parsing_request.empty())
		throw std::invalid_argument("Bad request");
	size_t pos = parsing_request.find(" ");
	if (pos == std::string::npos)
		throw std::invalid_argument("Bad request");
	url = parsing_request.substr(0, pos);
	if (url[0] != '/') //Not a bad request? Check behavior described in Discord by Maragao !
		throw std::invalid_argument("Bad request");
    if (url.size() > 1 && url[url.size() - 1] == '/')
        url = url.substr(0, url.size() - 1);
	parsing_request = parsing_request.substr(pos + 1);
	return url;
}

std::string RequestParser::getVersion(std::string &parsing_request)
{
	std::string version;

	if(parsing_request.empty())
		throw std::invalid_argument("Bad request");
	size_t pos = parsing_request.find("\r\n");
	if (pos == std::string::npos)
		throw std::invalid_argument("Bad request");
	version = parsing_request.substr(0, pos);
	if (version != "HTTP/1.1")
		throw std::invalid_argument("Bad request");
	parsing_request = parsing_request.substr(pos + 2);
	return version;
}

std::map<std::string, std::string> RequestParser::getHeaders(std::string &parsing_request)
{
	std::string strHeader;
	std::map<std::string, std::string> mapHeaders;

	if(parsing_request.empty())
		throw std::invalid_argument("Bad request");
	size_t pos = parsing_request.find("\r\n\r\n");
	if (pos == std::string::npos)
		throw std::invalid_argument("Bad request");
	strHeader = parsing_request.substr(0, pos + 2);
	parsing_request = parsing_request.substr(pos+4);

	std::string	line;
	int			line_end;
	size_t		pos_colon;
	while(!strHeader.empty())
	{
		line_end = strHeader.find("\r\n");
		if (line_end == std::string::npos)
			throw std::invalid_argument("Bad request");
		line = strHeader.substr(0, line_end);
		strHeader = strHeader.substr(line_end + 2);
		pos_colon = line.find(": ");
		if (pos_colon == std::string::npos)
			throw std::invalid_argument("Bad request");
		mapHeaders[line.substr(0, pos_colon)] = line.substr(pos_colon + 2);
	}
	return mapHeaders;
}

std::string RequestParser::getHost(std::string hostPort)
{
	size_t pos = hostPort.find(":");
	if (pos == std::string::npos)
		throw std::invalid_argument("Bad request");
	return hostPort.substr(0, pos);
}

std::string RequestParser::getPort(std::string hostPort)
{
	size_t pos = hostPort.find(":");
	return hostPort.substr(pos + 1);
}

std::string RequestParser::getBody(std::string &parsing_request)
{
	std::string body;

	if(parsing_request.empty()){
		return "";
	}
	if (parsing_request.substr(parsing_request.size() - 2) != std::string("\r\n")) //TEST: é necessário a request terminar com \r\n, e realmente só um é desconsiderado ? Testar com nginx e insomnia
		throw std::invalid_argument("Bad request");
	body = parsing_request.substr(0, parsing_request.size() - 2);
	parsing_request.clear();
	return body;
}

httpMethod RequestParser::stringToHttpMethod(const std::string& method) {
		if (method == "GET") return GET;
		if (method == "POST") return POST;
		if (method == "DELETE") return DELETE;
		throw std::invalid_argument("Invalid HTTP method");
}

std::string RequestParser::httpMethodToString(const httpMethod &method) {
	switch (method) {
		case GET: return "GET";
		case POST: return "POST";
		case DELETE: return "DELETE";
		default: throw std::invalid_argument("Invalid HTTP method");
	}
}