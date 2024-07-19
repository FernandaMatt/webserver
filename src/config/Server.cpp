#include "Server.hpp"

Server::Server() {
	this->_client_max_body_size = -1;
	this->_timeout = -1;
}

Server::~Server() {
}

Server& Server::operator=(const Server &obj){
	if (this != &obj )
	{
		_listeners = obj._listeners;
		_root = obj._root;
		_client_max_body_size = obj._client_max_body_size;
		_autoindex = obj._autoindex;
		_server_name = obj._server_name;
		_index = obj._index;
		_error_pages = obj._error_pages;
		_locations = obj._locations;
		_host_port = obj._host_port;
	}
	return(*this);
}

//setters
void    Server::set_listeners(std::string listen) {
	Listen listener;
	size_t colon_pos = listen.find_last_of(':');

	if (colon_pos != std::string::npos) {
		std::string host = listen.substr(0, colon_pos);
		std::string port = listen.substr(colon_pos + 1);
		if (host.find(':') != std::string::npos) { // ipv6
			if (host[0] != '[' && host[host.size() - 1] != ']')
				throw std::runtime_error("Error in config file: invalid ipv6 address format");
			host = host.substr(1, host.size() - 2);
		}
		if (host == "localhost")
			host = "127.0.0.1";
		check_host(host);
		check_port(port);
		check_duplicate_listen(host, port);
		listener = {host, port};
		_listeners.push_back(listener);
	}
	else {
		check_port(listen);
		check_duplicate_listen("0.0.0.0", listen);
		listener = {"0.0.0.0", listen};
		_listeners.push_back(listener);
	}
}

void    Server::check_port(std::string port) {
	for (size_t i = 0; i < port.size(); i++) {
		if (!std::isdigit(port[i]))
			throw std::runtime_error("Error in config file: invalid port " + port);
	}
	int port_int = atoi(port.c_str());
	if (port_int < 0 || port_int > 65535)
		throw std::runtime_error("Error in config file: invalid port " + port);
}

void    Server::check_host(std::string host) {
	if (host.find(' ') != std::string::npos)
		throw std::runtime_error("Error in config file: invalid host " + host);
	for (size_t i = 0; i < host.size(); i++) {
		if (!std::isdigit(host[i]) && host[i] != '.' && host[i] != ':')
			throw std::runtime_error("Error in config file: invalid host " + host);
	}
}

void    Server::set_root(std::string root) {
    if (!_root.empty())
        throw std::runtime_error("Error in config file: duplicate directive root");
    if (root.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid number of arguments in root");
    if (!root.empty() && root.size() != 1 && root[root.size() - 1] == '/')
        root.erase(root.size() - 1);
    _root = root;
}

void    Server::set_client_max_body_size(std::string client_max_body_size) {
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

void    Server::set_timeout(std::string timeout) {
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


void    Server::set_autoindex(std::string autoindex) {
	if (!_autoindex.empty())
		throw std::runtime_error("Error in config file: duplicate directive autoindex");
	if (autoindex.find(' ') != std::string::npos)
		throw std::runtime_error("Error in config file: invalid number of arguments in autoindex");
	if (autoindex != "on" && autoindex != "off")
		throw std::runtime_error("Error in config file: invalid autoindex directive");
	_autoindex = autoindex;
}

void    Server::set_server_name(std::string server_name) {
	size_t pos = 0;
	while (pos < server_name.size()) {
		size_t space_pos = server_name.find(' ', pos);
		if (space_pos != std::string::npos) {
			if (server_name.substr(pos, space_pos - pos).find('/') != std::string::npos ||
				server_name.substr(pos, space_pos - pos).find('\\') != std::string::npos) {
				Logger::log(LOG_WARNING, "server_name " + server_name.substr(pos, space_pos - pos) + " has suspicious symbols");
				pos = space_pos + 1;
				continue ;
			}
			_server_name.push_back(server_name.substr(pos, space_pos - pos));
			pos = space_pos + 1;
		}
		else {
			if (server_name.substr(pos).find('/') != std::string::npos ||
				server_name.substr(pos).find('\\') != std::string::npos)
				Logger::log(LOG_WARNING, "server_name " + server_name.substr(pos) + " has suspicious symbols");
			else
				_server_name.push_back(server_name.substr(pos));
			break;
		}
	}
}

void    Server::set_index(std::string index) {
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

void    Server::set_error_page(std::string error_page) {
	size_t  pos_space;

	pos_space = error_page.find(' ');
	if (pos_space == std::string::npos)
		throw std::runtime_error("Error in config file: invalid location error page");

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
		it->second = error_path;
	else
		_error_pages[error_code] = error_path;
}

void    Server::set_location(Location &location) {
	check_duplicate_location(location);
	_locations.push_back(location);
}

void    Server::set_host_port(const std::string &hostPort) {
	this->_host_port = hostPort;
}

//getters

const std::vector<Listen>&  Server::get_listeners() const {return _listeners;}

std::string&    Server::get_root() {return _root;}

long    Server::get_client_max_body_size() {return _client_max_body_size;}

int	Server::get_timeout() {return _timeout;}

std::string&    Server::get_autoindex() {return _autoindex;}

std::vector<std::string>&   Server::get_server_name() {return _server_name;}

std::vector<std::string>&   Server::get_index() {return _index;}

std::map<int, std::string>& Server::get_error_pages() {return _error_pages;}

std::string     Server::get_error_page_path(int error_code) {
    std::map<int, std::string>::iterator it = _error_pages.find(error_code);
    if (it == _error_pages.end())
        return NULL;
    return it->second;
}

std::vector<Location>&  Server::get_location() {return _locations;}

std::string Server::get_host_port() {return this->_host_port;}

//methods

void    Server::check_duplicate_location(Location &location) {
    for(size_t i = 0; i < _locations.size(); i++) {
        if (location.get_path() == _locations[i].get_path())
            throw std::runtime_error("Error in config file: duplicate location path " + location.get_path());
    }
}

void    Server::check_duplicate_listen(std::string host, std::string port) {
	for(size_t i = 0; i < _listeners.size(); i++){
		if (host == _listeners[i].host && port == _listeners[i].port)
			throw std::runtime_error("Error in config file: duplicate listen: " + host + ":" + port);
	}
}

void    Server::set_default_directives(){

    if (_listeners.empty())
        set_listeners("0.0.0.0:80");
    if (_index.empty())
        set_index("index.html");
    if (_autoindex.empty())
        set_autoindex("off");
    if (_client_max_body_size == -1)
        set_client_max_body_size("1m");
	if (_timeout == -1)
		set_timeout("60s");
    if (_root.empty())
        set_root("/");

    bool found = false;
    for (size_t i = 0; i < _locations.size(); i++) {
        if (_locations[i].get_path() == "/")
            found = true;
    }
    if (found == false) {
        Location location;
        location.set_path("/");
        this->set_location(location);
    }
	for (std::map<int, std::string>::iterator it = _error_pages.begin(); it != _error_pages.end(); it++) {
		if (it->second[0] != '/') {
			if (_root[_root.size() - 1] != '/') {
				std::string aux = _root + '/' + it->second;
				it->second.clear();
				it->second = aux;
			}
			else {
				std::string aux = _root + it->second;
				it->second.clear();
				it->second = aux;
			}
		}
    }

    //set default directives for locations
    for (size_t i = 0; i < _locations.size(); i++) {

        if (_locations[i].get_root().empty() && _locations[i].get_alias().empty())
            _locations[i].set_root(_root);
        if (_locations[i].get_autoindex().empty())
            _locations[i].set_autoindex(_autoindex);
        if (_locations[i].get_client_max_body_size() == -1) {
            std::stringstream ss_body_size;
            ss_body_size << _client_max_body_size;
            _locations[i].set_client_max_body_size(ss_body_size.str());
        }
		if (_locations[i].get_timeout() == -1) {
            std::stringstream ss_timeout;
            ss_timeout << _timeout;
            _locations[i].set_timeout(ss_timeout.str());
        }
        if (_locations[i].get_index().empty()) {
            std::string index;
            for (size_t j = 0; j < _index.size(); j++) {
                index += _index[j];
                if (j < _index.size() - 1)
                    index += " ";
            }
            _locations[i].set_index(index);
        }
        if (_locations[i].get_methods().empty()) {
            _locations[i].set_methods("GET");
        }
		for (std::map<int, std::string>::iterator it = _locations[i].get_error_pages().begin(); it != _locations[i].get_error_pages().end(); it++) {
			if (it->second[0] != '/') {
				if (!_locations[i].get_root().empty()) {
					if (_locations[i].get_root()[_locations[i].get_root().size() - 1] != '/') {
						std::string aux = _locations[i].get_root() + '/' + it->second;
						it->second.clear();
						it->second = aux;
					}
					else {
						std::string aux = _locations[i].get_root() + it->second;
						it->second.clear();
						it->second = aux;
					}
				}
				else {
					if (_locations[i].get_alias()[_locations[i].get_alias().size() - 1] != '/') {
						std::string aux = _locations[i].get_alias() + '/' + it->second;
						it->second.clear();
						it->second = aux;
					}
					else {
						std::string aux = _locations[i].get_alias() + it->second;
						it->second.clear();
						it->second = aux;
					}
				}
			}
		}
        std::map<int, std::string> error_page_map = _locations[i].get_error_pages();
		for (std::map<int, std::string>::iterator it = _error_pages.begin(); it != _error_pages.end(); it++) {
            if (error_page_map.find(it->first) == error_page_map.end()) {
                std::stringstream ss;
                ss << it->first;
                _locations[i].set_error_page(ss.str() + " " + it->second);
            }
        }
        if (_locations[i].get_upload_path().empty()) {
            if (!(_locations[i].get_root().empty()))
                _locations[i].set_upload_path(_locations[i].get_root());
            else
                _locations[i].set_upload_path(_locations[i].get_alias());
        }
	}    
}

void    Server::print_all_directives() const {

	for(size_t i = 0; i < _listeners.size(); i++){
		std::cout << BLUE << "listen: " << _listeners[i].host << ":" << RST << _listeners[i].port << std::endl;
	}
    std::cout << BLUE << "root: " << RST << _root << std::endl;
    std::cout << BLUE << "client_max_body_size: " << RST << _client_max_body_size << std::endl;
    std::cout << BLUE << "timeout: " << RST << _timeout << std::endl;
	std::cout << BLUE << "autoindex: " << RST << _autoindex << std::endl;
    std::cout << BLUE << "server_name [ size = " << _server_name.size() << " ]: ";
    for (size_t i = 0; i < _server_name.size(); i++) {
        std::cout << _server_name[i] << " ";
    }
    std::cout << std::endl;
    std::cout << BLUE << "index [ size = " << _index.size() << " ]: ";
    for (size_t i = 0; i < _index.size(); i++) {
        std::cout << RST << _index[i] << " ";
    }

    std::cout << std::endl;
    std::map<int, std::string>::const_iterator it;
    for (it = _error_pages.begin(); it != _error_pages.end(); it++) {
        std::cout << BLUE << "error_page [ " << it->first << " ]: " << RST << it->second << std::endl;
    }
    std::cout << std::endl;
    std::cout << BLUE << "locations [ size = " << _locations.size() << BLUE << " ]" << RST << std::endl;
    for (size_t i = 0; i < _locations.size(); i++) {
        std::cout << BLUE << "location " << RST << _locations[i].get_path() << " {" << std::endl;
        _locations[i].print_all_directives();
    }

	std::cout << BLUE << "host_port: " << RST << this->_host_port << std::endl;
    std::cout << std::endl;
}
