#include "Location.hpp"

Location::Location() {
    _client_max_body_size = -1;
    _timeout = -1;
    _redirect = false;
}

Location::~Location() {
}

Location& Location::operator=(const Location &obj){
	if (this != &obj )
	{
        _path = obj._path;
        _root = obj._root;
		_alias = obj._alias;
		_client_max_body_size = obj._client_max_body_size;
		_autoindex = obj._autoindex;
		_index = obj._index;
        _methods = obj._methods;
		_error_pages = obj._error_pages;
        _upload_path = obj._upload_path;
        _cgi_path = obj._cgi_path;
        _cgi_ext = obj._cgi_ext;
        _redirect = obj._redirect;
        _redirect_code = obj._redirect_code;
        _redirect_path = obj._redirect_path;
    }
	return(*this);
}


//setters

void    Location::set_path(std::string path) {
    if (path.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in location path");
    if (!path.empty() && path.size() != 1 && path[path.size() - 1] == '/')
        path.erase(path.size() - 1);
    _path = path;
}

void    Location::set_root(std::string root) {
    if (!_alias.empty())
        throw std::runtime_error("Error in config file: invalid location root directive (alias is already setted)");
    if (!_root.empty())
        throw std::runtime_error("Error in config file: duplicate directive root");
    if (root.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in root");
    if (!root.empty() && root.size() != 1 && root[root.size() - 1] == '/')
        root.erase(root.size() - 1);
    _root = root;
}

void    Location::set_index(std::string index) {
    size_t pos = 0;
    while (pos < index.size()) {
        size_t space_pos = index.find(' ', pos);
        std::string token;
        if (space_pos != std::string::npos){
            token = index.substr(pos, space_pos - pos);
            pos = space_pos + 1;
        }
        else {
            token = index.substr(pos);
            pos = index.size();
        }
        if (!token.empty() && token.size() != 1 && token[0] == '/')
            token = token.substr(1);
        _index.push_back(token);
    }
}

void    Location::set_autoindex(std::string autoindex) {
    if (!_autoindex.empty())
        throw std::runtime_error("Error in config file: duplicate directive autoindex");
    if (autoindex.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in autoindex");
    if (autoindex != "on" && autoindex != "off")
        throw std::runtime_error("Error in config file: invalid location autoindex " + autoindex);
    _autoindex = autoindex;
}

void    Location::set_alias(std::string alias) {
    if (!_root.empty())
        throw std::runtime_error("Error in config file: invalid location alias directive (root is already setted)");
    if (!_alias.empty())
        throw std::runtime_error("Error in config file: duplicate directive alias");
    if (alias.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in alias");
    if (!alias.empty() && alias.size() != 1 && alias[alias.size() - 1] == '/')
        alias.erase(alias.size() - 1);
    _alias = alias;
}

void    Location::set_client_max_body_size(std::string client_max_body_size) {
	bool has_suffix_m = false;

	if (_client_max_body_size != -1)
		throw std::runtime_error("Error in config file: duplicate directive client_max_body_size");
	if (client_max_body_size[client_max_body_size.size() - 1] == 'M' || client_max_body_size[client_max_body_size.size() - 1] == 'm')
		has_suffix_m = true;
	for (size_t pos = 0; pos < client_max_body_size.size(); pos++) {
		if (has_suffix_m == false) {
			if (!std::isdigit(client_max_body_size[pos]))
				throw std::runtime_error("Error in config file: invalid client max body size");
		} else {
			if (pos != client_max_body_size.size() - 1 && !std::isdigit(client_max_body_size[pos]))
				throw std::runtime_error("Error in config file: invalid client max body size");
		}
	}

	char *end;
	errno = 0;
	long result = std::strtol(client_max_body_size.c_str(), &end, 10);
	if (errno == ERANGE) {
        if (result == LONG_MAX) {
            throw std::runtime_error("Error: client max body size too large");
        } else if (result == LONG_MIN) {
            throw std::runtime_error("Error: client max body size too small");
        }
    }
    if (*end != '\0' && *end != 'M' && *end != 'm')
        throw std::runtime_error("Error: invalid character in client max body size");
	if (has_suffix_m == true) 
		result *= 1048576;
	_client_max_body_size = result;	
}

void    Location::set_timeout(std::string timeout) {
	bool has_suffix_s = false;

    if (_timeout != -1)
        throw std::runtime_error("Error in config file: duplicate directive keepalive_timeout");
    if (timeout[timeout.size() - 1] == 's')
        has_suffix_s = true;
	for (size_t pos = 0; pos < timeout.size(); pos++) {
		if (has_suffix_s == false) {
			if (!std::isdigit(timeout[pos]))
				throw std::runtime_error("Error in config file: invalid keepalive_timeout");
		} else {
			if (pos != timeout.size() - 1 && !std::isdigit(timeout[pos]))
				throw std::runtime_error("Error in config file: invalid keepalive_timeout");
		}
	}

    char* end;
    errno = 0;
    long result = std::strtol(timeout.c_str(), &end, 10);
    if (errno == ERANGE) {
        if (result == LONG_MAX) {
            throw std::runtime_error("Error: timeout value too large");
        } else if (result == LONG_MIN) {
            throw std::runtime_error("Error: timeout value too small");
        }
    }
    if (*end != '\0' && *end != 's')
        throw std::runtime_error("Error: invalid character in keepalive_timeout");
    if (result < 0 || result > INT_MAX)
        throw std::runtime_error("Error: timeout value out of range");
    _timeout = static_cast<int>(result);
}

void    Location::set_methods(std::string methods) {
    if (!_methods.empty())
        throw std::runtime_error("Error in config file: duplicate directive allow_methods");
    for (size_t pos = 0; pos < methods.size(); pos++) {
        size_t space_pos = methods.find(' ', pos);
        std::string method;
        if (space_pos != std::string::npos){
            method = methods.substr(pos, space_pos - pos);
            pos = space_pos;
        }
        else {
            method = methods.substr(pos);
            pos = methods.size();
        }
        if (method != "GET" && method != "POST" && method != "DELETE" &&
            method != "get" && method != "post" && method != "delete") {
            throw std::runtime_error("Error in config file: invalid methods" + method); }
        _methods.push_back(method);
    }
}

void    Location::set_error_page(std::string error_page) {
    size_t  pos_space;

    pos_space = error_page.find(' ');
    if (pos_space == std::string::npos)
        throw std::runtime_error("Error in config file: invalid location error page arguments");

    std::string error_code_str = error_page.substr(0, pos_space);
    for (size_t i = 0; i < error_code_str.size(); i++) {
        if (!std::isdigit(error_code_str[i]))
            throw std::runtime_error("Error in config file: invalid location error page");
    }
    int error_code = atoi(error_code_str.c_str());
    if (error_code < 400 || error_code > 599)
        throw std::runtime_error("Error in config file: invalid location error page");

    std::string error_path = error_page.substr(pos_space + 1);
    if (error_path.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid location error page");

    std::map<int, std::string>::iterator it = _error_pages.find(error_code);
    if (it != _error_pages.end())
        it ->second = error_path;
    else
        _error_pages[error_code] = error_path;
}

void    Location::set_upload_path(std::string upload_path) {
    if (!_upload_path.empty())
        throw std::runtime_error("Error in config file: duplicate directive upload_path");
    if (upload_path.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in upload_path");
    if (!upload_path.empty() && upload_path.size() != 1 && upload_path[upload_path.size() - 1] == '/')
        upload_path.erase(upload_path.size() - 1);
    _upload_path = upload_path;
}

void    Location::set_cgi_path(std::string cgi_path) {
    if (!_cgi_path.empty())
        throw std::runtime_error("Error in config file: duplicate directive cgi_path");
    if (cgi_path.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in cgi_path");
    _cgi_path = cgi_path;
}

void    Location::set_cgi_ext(std::string cgi_ext) {
    if (!_cgi_ext.empty())
        throw std::runtime_error("Error in config file: duplicate directive cgi_ext");
    if (cgi_ext.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in cgi_ext");
    _cgi_ext = cgi_ext;
}

void    Location::set_redirect(std::string redirect) {
    if (_redirect)
        throw std::runtime_error("Error in config file: duplicate directive redirect");
    size_t pos_space = redirect.find(' ');
    if (pos_space == std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in redirect");
    _redirect = true;
    _redirect_code = redirect.substr(0, pos_space);
    if (_redirect_code != "300" && _redirect_code != "301" && _redirect_code != "302"
        && _redirect_code != "303" && _redirect_code != "304" && _redirect_code != "305"
        && _redirect_code != "307" && _redirect_code != "308")
        throw std::runtime_error("Error in config file: invalid redirect code");
    _redirect_path = redirect.substr(pos_space + 1);
    if (_redirect_path.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in redirect");
}

//getters
std::string                 Location::get_path() const {return _path;}

std::string&    Location::get_root() {return _root;}

std::string&    Location::get_alias() {return _alias;}

long    Location::get_client_max_body_size() {return _client_max_body_size;}

int	Location::get_timeout() {return _timeout;}


std::string&    Location::get_autoindex() {return _autoindex;}

std::vector<std::string>&   Location::get_index() {return _index;}

std::vector<std::string>&   Location::get_methods() {return _methods;}

std::map<int, std::string>& Location::get_error_pages() {return _error_pages;}

std::string     Location::get_error_page_path(int error_code) {
    std::map<int, std::string>::iterator it = _error_pages.find(error_code);
    if (it == _error_pages.end())
        return NULL;
    return it->second;
}

std::string&    Location::get_upload_path() {return _upload_path;}

std::string&    Location::get_cgi_path() {return _cgi_path;}

std::string&    Location::get_cgi_ext() {return _cgi_ext;}

bool    Location::get_redirect() {return _redirect;}

std::string&    Location::get_redirect_path() {return _redirect_path;}

std::string&    Location::get_redirect_code() {return _redirect_code;}

void    Location::print_all_directives() const {

    std::cout << GRN << "      path: " << RST << _path << std::endl;
    std::cout << GRN << "      root: " << RST << _root << std::endl;
    std::cout << GRN << "      alias: " << RST << _alias << std::endl;
    std::cout << GRN << "      client_max_body_size: " << RST << _client_max_body_size << std::endl;
    std::cout << GRN << "      timeout " << RST << _timeout << std::endl;
    std::cout << GRN << "      autoindex " << RST << _autoindex << std::endl;
    std::cout << GRN << "      index [ size = " << _index.size() << " ]: ";
    for (size_t i = 0; i < _index.size(); i++) {
        std::cout << RST << _index[i] << " ";
    }
    std::cout << std::endl;
    std::cout << GRN << "      methods [ size = " << _methods.size() << " ]: ";
    for (size_t i = 0; i < _methods.size(); i++) {
        std::cout << RST << _methods[i] << " ";
    }
    std::cout << std::endl;
    std::map<int, std::string>::const_iterator it;
    for (it = _error_pages.begin(); it != _error_pages.end(); it++) {
        std::cout << GRN << "      error_page [ " << it->first << " ]: " << RST << it->second << std::endl;
    }
    std::cout << GRN << "      upload_path: " << RST << _upload_path << std::endl;
    std::cout << GRN << "      cgi_path: " << RST << _cgi_path << std::endl;
    std::cout << GRN << "      cgi_ext: " << RST << _cgi_ext << std::endl;
    std::cout << std::endl;
}
std::string Location::search_index_file(std::string path) {
    struct stat buffer;
    std::string index_file_path;
    std::string file_name; //move manipulation of '/' to server_parser

    for (size_t i = 0; i < _index.size(); i++) {
        file_name = _index[i];
        if (file_name[0] == '/')
            file_name = file_name.substr(1);
        if (file_name[file_name.size() - 1] == '/')
            file_name = file_name.substr(0, file_name.size() - 1);
        if (path[path.size() - 1] == '/')
            path = path.substr(0, path.size() - 1);
        index_file_path = path + '/' +file_name;
        if (stat(index_file_path.c_str(), &buffer) == 0)
            return index_file_path;
    }
    return "";
}
