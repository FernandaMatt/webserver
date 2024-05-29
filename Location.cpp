#include "Location.hpp"

Location::Location() {
    _client_max_body_size = 1000000;
    _autoindex = "off";
}

Location::~Location() {
    
}

//setters

void    Location::set_path(std::string path) {
    _path = path;
}

void    Location::set_root(std::string root) {
    if (!_alias.empty())
        throw std::runtime_error("Error in config file: invalid location root directive (alias is already setted)");    
    _root = root;
}

void    Location::set_index(std::string index) {
    size_t pos = 0;
    while (pos < index.size()) {
        size_t space_pos = index.find(' ', pos);
        if (space_pos != std::string::npos){
            _index.push_back(index.substr(pos, space_pos - pos));
            pos = space_pos + 1;
        }
        else {
            _index.push_back(index.substr(pos));
            break;
        }
    }
}

void    Location::set_autoindex(std::string autoindex) {
    if (autoindex != "on" && autoindex != "off") 
        throw std::runtime_error("Error in config file: invalid location autoindex " + autoindex);
    _autoindex = autoindex;
}

void    Location::set_alias(std::string alias) {
    if (!_root.empty())
        throw std::runtime_error("Error in config file: invalid location alias directive (root is already setted)");    
    _alias = alias;
}

void    Location::set_client_max_body_size(std::string client_max_body_size) {
    bool mb = false;

    if (client_max_body_size[client_max_body_size.size() - 1] == 'M' || client_max_body_size[client_max_body_size.size() - 1] == 'm')
        mb = true;
    for (size_t pos = 0; pos < client_max_body_size.size(); pos++) {
        if (mb == false) {
            if (!std::isdigit(client_max_body_size[pos]))
                throw std::runtime_error("Error in config file: invalid location client max body size");
        } else {
            if (pos != client_max_body_size.size() - 1 && !std::isdigit(client_max_body_size[pos]))
                throw std::runtime_error("Error in config file: invalid location client max body size");
        }
    }
    if (mb == true) _client_max_body_size = std::strtoul(client_max_body_size.c_str(), NULL, 0) * 1000000;
    else _client_max_body_size = std::strtoul(client_max_body_size.c_str(), NULL, 0);
}

void    Location::set_methods(std::string methods) {
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
    size_t  pos_semicolon;
    size_t  pos_space;

    pos_semicolon = error_page.find(';');
    if (pos_semicolon == std::string::npos)
        throw std::runtime_error("Error in config file: invalid location error page");
    
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

    std::string error_path = error_page.substr(pos_space + 1, pos_semicolon - pos_space - 1);
    if (error_path.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid location error page");         
    _error_pages[error_code] = error_path;
}

//getters
std::string                 Location::get_path() {return _path;}

std::string                 Location::get_root() {return _root;}

std::string                 Location::get_alias() {return _alias;}

unsigned long               Location::get_client_max_body_size() {return _client_max_body_size;}

std::string                 Location::get_autoindex() {return _autoindex;}

std::vector<std::string>    Location::get_index() {return _index;}

std::vector<std::string>    Location::get_methods() {return _methods;}

std::map<int, std::string>  Location::get_error_pages() {return _error_pages;}

std::string                 Location::get_error_page_path(int error_code) {
    std::map<int, std::string>::iterator it = _error_pages.find(error_code);
    if (it == _error_pages.end())
        throw std::runtime_error("Error page not found: error code " + error_code);
    return it->second;
}

void    Location::check_directives() {

}

void    Location::print_all_directives() {
    
    std::cout << "      path: " << _path << std::endl; 
    std::cout << "      root: " << _root << std::endl; 
    std::cout << "      alias: " << _alias << std::endl; 
    std::cout << "      client_max_body_size: " << _client_max_body_size << std::endl;
    std::cout << "      autoindex " << _autoindex << std::endl; 
    std::cout << "      index [ size = " << _index.size() << " ]: ";
    for (size_t i = 0; i < _index.size(); i++) {
        std::cout << _index[i] << " "; 
    }
    std::cout << std::endl;
    std::cout << "      methods [ size = " << _methods.size() << " ]: ";
    for (size_t i = 0; i < _methods.size(); i++) {
        std::cout << _methods[i] << " "; 
    }
    std::cout << std::endl;
    std::map<int, std::string>::iterator it;
    for (it = _error_pages.begin(); it != _error_pages.end(); it++) {
        std::cout << "      error_page [ " << it->first << " ]: " << it->second << std::endl;
    }
    std::cout << std::endl;
}