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
    req.queryVariables.clear();
    req.extraPath.clear();
	try {
		req.method = getMethod(parsing_request);
		req.path = getPath(parsing_request);
        req.type = getResourceType(req.path);
        if (req.type == "CGI") {
            req.extraPath = getExtraPath(req.path);
            req.CGIfilename = getCGIfilename(req.path);
            req.path = getCGIPath(req.path);
        }
        if (parsing_request[0] == '?') {
            req.queryString = getQueryString(parsing_request);
            req.queryVariables = getQueryVariables(parsing_request);
        }
		req.version = getVersion(parsing_request);
		req.headers = getHeaders(parsing_request);
		req.body = getBody(parsing_request);
		req.host = getHost(req.headers["Host"]);
		req.port = getPort(req.headers["Host"]);
		req.request_status = getRequestStatus(req);
	} catch (std::exception &e) {
		if (std::string(e.what()) == "Bad request")
			req.statusCode = 400;
		if (std::string(e.what()) == "Method not allowed")
			req.statusCode = 405;
		return req;
	}
	req.statusCode = 200;
    //req.printRequest();
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
    if (url.find("?") != std::string::npos) {
        pos = url.find("?");
        url = url.substr(0, pos);
        pos = parsing_request.find("?") - 1;
        // parsing_request = parsing_request.substr(pos);
    }
    if (url.size() > 1 && url[url.size() - 1] == '/')
        url = url.substr(0, url.size() - 1);
	parsing_request = parsing_request.substr(pos + 1);
	return url;
}

std::string RequestParser::getResourceType(std::string url)
{
    if (url.find(".php") != std::string::npos)
        return "CGI";
    return "STATIC";
}

std::string RequestParser::getCGIfilename(std::string url)
{
    std::string filename;
    size_t pos = url.find(".php");
    if (pos == std::string::npos)
        throw std::invalid_argument("Bad request");
    filename = url.substr(0, pos + 4);
    pos = filename.rfind("/");
    return filename.substr(pos + 1, filename.size() - pos - 1);
}

std::string RequestParser::getExtraPath(std::string url)
{
    size_t pos = url.find(".php");
    if (pos == std::string::npos)
        throw std::invalid_argument("Bad request");
    return url.substr(pos + 4, url.size() - pos - 4);
}

std::string RequestParser::getCGIPath(std::string url)
{
    size_t pos = url.find(".php");
    if (pos == std::string::npos)
        throw std::invalid_argument("Bad request");
    std::string path_file = url.substr(0, pos);
    pos = path_file.rfind("/");
    return path_file.substr(0, pos);
}

std::string RequestParser::getQueryString(std::string &parsing_request)
{
    size_t pos;

    if(parsing_request.empty())
        throw std::invalid_argument("Bad request");
    pos = parsing_request.find(" ");
    if (pos == std::string::npos)
        throw std::invalid_argument("Bad request");
    return parsing_request.substr(1, pos - 1);
}

std::map<std::string, std::string> RequestParser::getQueryVariables(std::string &parsing_request)
{
    std::map<std::string, std::string> queryVariables;
    std::string query;
    size_t pos;

    if(parsing_request.empty())
        throw std::invalid_argument("Bad request");
    pos = parsing_request.find(" ");
    if (pos == std::string::npos)
        throw std::invalid_argument("Bad request");
    query = parsing_request.substr(1, pos - 1);
    parsing_request = parsing_request.substr(pos + 1);
    pos = query.find("&");
    while (pos != std::string::npos)
    {
        queryVariables[query.substr(0, query.find("="))] = query.substr(query.find("=") + 1, query.find("&") - query.find("=") - 1);
        query = query.substr(query.find("&") + 1);
        pos = query.find("&");
    }
    queryVariables[query.substr(0, query.find("="))] = query.substr(query.find("=") + 1);
    return queryVariables;
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
	// if (parsing_request.substr(parsing_request.size() - 2) != std::string("\r\n")) //TEST: é necessário a request terminar com \r\n, e realmente só um é desconsiderado ? Testar com nginx e insomnia
	// 	throw std::invalid_argument("Bad request");
	body = parsing_request.substr(0, parsing_request.size() - 2);
	parsing_request.clear();
	return body;
}


std::string	RequestParser::getRequestStatus(httpRequest &request) {
	if (isChunkedBody(request))
		return isChunkedComplete(request.body);
	else if (isMultipartBody(request))
		return isMultipartComplete(request);
	return "complete";
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

bool    RequestParser::isChunkedBody(httpRequest &request) {
    if (request.headers.find("Transfer-Encoding") != request.headers.end()) {
        if (request.headers.find("Transfer-Encoding")->second == "chunked")
            return true;
    }
    return false;
}


std::string	RequestParser::isChunkedComplete(std::string &body) {
	if (body.empty())
        return "incomplete";
	size_t end_chunked = body.find("\r\n0\r\n");
	if (end_chunked == std::string::npos)
		return "incomplete";
	if (end_chunked == body.size() - 5)
        return "complete";
    return "incomplete";
}

bool    RequestParser::isMultipartBody(httpRequest &request) {
    if (request.headers.find("Content-Type") != request.headers.end()) {
        std::string content_type = request.headers.find("Content-Type")->second;
        if (content_type.compare(0, 30, "multipart/form-data; boundary=") == 0)
                return true;
    }
    return false;
}

std::string	RequestParser::isMultipartComplete(httpRequest &request) {
	if (request.body.empty())
        return "incomplete";
    std::string content_type = request.headers.find("Content-Type")->second;
    size_t pos = content_type.find('=');
    if (pos == std::string::npos)
        return "incomplete";
    std::string boundary = content_type.substr(pos + 1) + "--";
	size_t end_boundary = request.body.find(boundary);
	if (end_boundary == std::string::npos)
		return "incomplete";
	if (request.body.size() == end_boundary + boundary.size())
		return "complete";
	return "incomplete";
}
