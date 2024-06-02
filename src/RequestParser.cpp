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
	httpRequest req;
	req.method = getMethod(request);
	// req.path = getPath(request);
	// req.version = getVersion(request);
	// req.headers = getHeaders(request);
	// req.body = getBody(request);
	return req;
}

httpMethod RequestParser::getMethod(std::string request)
{
	std::string method;
	size_t pos = request.find(" ");
	method = request.substr(0, pos);
	return (stringToHttpMethod(method));
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