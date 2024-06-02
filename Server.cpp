#include "Server.hpp"

Server::Server() {
    this->_client_max_body_size = 1000000;
    this->_autoindex = "off";
    this->_sock_fd = -1;

}

Server::~Server() {

}

//void    Server::init_error_pages() {
    //4XX Erros do cliente
    //400 Bad Request - sintaxe invalida
    //401 Unauthorized - requer autorização de usuario
    //403 Forbidden - nao tem permissão
    //404 Not Found - Servidor nao encontrou o recurso
    //405 Method Not Allowed - método nao é permitido
    //408 Request Timeout - Servidor atingiu o tempo limite

    //5XX Erros do servidor
    //500 Internal Server Error - erro interno no servidor
    //501 Not Implemented - servidor nao reconhece o metodo
    //502 Bad Gateway
    //503 Service Unavailable - sobrecarga ou manutencao do servidor
    //504 Gateway Timeout - nao recebeu resposta a tempo
    //505 HTTP Version Not Supported - nao suporta a versao HTTP
//}

//setters

void    Server::set_listen(std::string listen) {
    size_t colon_pos = listen.find_last_of(':');
    if (colon_pos != std::string::npos) {
        std::string host = listen.substr(0, colon_pos);
        std::string port = listen.substr(colon_pos + 1);
        if (host.find(':') != std::string::npos) { //endereco ipv6
            if (host[0] != '[' && host[host.size() - 1] != ']')
                throw std::runtime_error("Error in config file: invalid ipv6 address format");
            host = host.substr(1, host.size() - 2);
        }
        set_host(host);
        set_port(port);
    }
    else {
        set_port(listen);
        set_host("127.0.0.1");
    }
}

void    Server::set_port(std::string port) {
    for (size_t i = 0; i < port.size(); i++) {
        if (!std::isdigit(port[i]))
            throw std::runtime_error("Error in config file: invalid port " + port);
    }
    int port_int = atoi(port.c_str());
    if (port_int < 0 || port_int > 65535)
        throw std::runtime_error("Error in config file: invalid port " + port);
    _port = port;
}

void    Server::set_host(std::string host) {
    if (host.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid host " + host);
    for (size_t i = 0; i < host.size(); i++) {
        if (!std::isdigit(host[i]) && host[i] != '.' && host[i] != ':')
            throw std::runtime_error("Error in config file: invalid host " + host);
    }
    if (host == "localhost") _host = "127.0.0.1";
    else _host = host;
}

void    Server::set_root(std::string root) {_root = root;}


void    Server::set_client_max_body_size(std::string client_max_body_size) {
    bool mb = false;

    if (client_max_body_size[client_max_body_size.size() - 1] == 'M' || client_max_body_size[client_max_body_size.size() - 1] == 'm')
        mb = true;
    for (size_t pos = 0; pos < client_max_body_size.size(); pos++) {
        if (mb == false) {
            if (!std::isdigit(client_max_body_size[pos]))
                throw std::runtime_error("Error in config file: invalid client max body size");
        } else {
            if (pos != client_max_body_size.size() - 1 && !std::isdigit(client_max_body_size[pos]))
                throw std::runtime_error("Error in config file: invalid client max body size");
        }
    }
    if (mb == true) _client_max_body_size = std::strtoul(client_max_body_size.c_str(), NULL, 0) * 1000000;
    else _client_max_body_size = std::strtoul(client_max_body_size.c_str(), NULL, 0);
}

void    Server::set_autoindex(std::string autoindex) {
    if (autoindex != "on" && autoindex != "off")
        throw std::runtime_error("Error in config file: invalid autoindex directive");
    _autoindex = autoindex;
}

void    Server::set_server_name(std::string server_name) {
    size_t pos = 0;
    while (pos < server_name.size()) {
        size_t space_pos = server_name.find(' ', pos);
        if (space_pos != std::string::npos){
            _server_name.push_back(server_name.substr(pos, space_pos - pos));
            pos = space_pos + 1;
        }
        else {
            _server_name.push_back(server_name.substr(pos));
            break;
        }
    }
}

void    Server::set_index(std::string index) {
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

//check if error code has only digits and if there are only one path
void    Server::set_error_page(std::string error_page) {
    size_t  pos_semicolon;
    size_t  pos_space;

    pos_semicolon = error_page.find(';');
    if (pos_semicolon == std::string::npos)
        throw std::runtime_error("Error in config file: invalid error page");

    pos_space = error_page.find(' ');
    if (pos_space == std::string::npos)
        throw std::runtime_error("Error in config file: invalid error page");

    std::string error_code_str = error_page.substr(0, pos_space);
    for (size_t i = 0; i < error_code_str.size(); i++) {
        if (!std::isdigit(error_code_str[i]))
            throw std::runtime_error("Error in config file: invalid error page");
    }
    int error_code = atoi(error_code_str.c_str());
    if (error_code < 400 || error_code > 599)
        throw std::runtime_error("Error in config file: invalid error page");

    std::string error_path = error_page.substr(pos_space + 1, pos_semicolon - pos_space - 1);
    if (error_path.find(' ') != std::string::npos)
        throw std::runtime_error("Error in config file: invalid error page");
    _error_pages[error_code] = error_path;
}

//check for methods GET POST DELETE
void    Server::set_methods(std::string methods) {
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

void    Server::set_location(Location &location) {
    location.check_directives();
    _locations.push_back(location);
}

void    Server::set_sock_fd() {

    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;
    int             status;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;

    status = getaddrinfo(this->_host.c_str(), this->_port.c_str(), &hints, &result);
    if (status != 0) {
        Logger::log(LOG_ERROR, gai_strerror(status));
        throw std::runtime_error("Error: getaddrinfo()");
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        this->_sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if(this->_sock_fd == -1)
            continue;

        int option = 1;
        if (setsockopt(this->_sock_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1){
            Logger::log(LOG_ERROR, "setsockopt() failure");
            close(this->_sock_fd);
            freeaddrinfo(result);
            throw std::runtime_error("Error: setsockopt()");
        }

        if (bind(this->_sock_fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break ;
    }

    if(rp == NULL){ //No address succeded
        Logger::log(LOG_ERROR, "Could not bind");
        freeaddrinfo(result);
        throw std::runtime_error("Error: bind()");
    }

    freeaddrinfo(result);
}

//getters
std::string     Server::get_port() {return _port;}

std::string     Server::get_host() {return _host;}

std::string     Server::get_root() {return _root;}

unsigned long    Server::get_client_max_body_size() {return _client_max_body_size;}

std::string    Server::get_autoindex() {return _autoindex;}

std::vector<std::string>     Server::get_server_name() {return _server_name;}

std::vector<std::string>    Server::get_index() {return _index;}

std::map<int, std::string>  Server::get_error_pages() {return _error_pages;}

std::string                 Server::get_error_page_path(int error_code) {
    std::map<int, std::string>::iterator it = _error_pages.find(error_code);
    if (it == _error_pages.end())
        throw std::runtime_error("Error page not found: error code " + error_code);
    return it->second;
}

std::vector<std::string>    Server::get_methods() {return _methods;}

std::vector<Location>    Server::get_location() {return _locations;}

int Server::get_sock_fd() const {return _sock_fd;}

//methods

void    Server::print_all_directives() {

    std::cout << "   port: " << _port << std::endl;
    std::cout << "   host: " << _host << std::endl;
    std::cout << "   root: " << _root << std::endl;
    std::cout << "   client_max_body_size: " << _client_max_body_size << std::endl;
    std::cout << "   autoindex " << _autoindex << std::endl;
    std::cout << "   server_name [ size = " << _server_name.size() << " ]: ";
    for (size_t i = 0; i < _server_name.size(); i++) {
        std::cout << _server_name[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "   index [ size = " << _index.size() << " ]: ";
    for (size_t i = 0; i < _index.size(); i++) {
        std::cout << _index[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "   methods [ size = " << _methods.size() << " ]: ";
    for (size_t i = 0; i < _methods.size(); i++) {
        std::cout << _methods[i] << " ";
    }
    std::cout << std::endl;
    std::map<int, std::string>::iterator it;
    for (it = _error_pages.begin(); it != _error_pages.end(); it++) {
        std::cout << "   error_page [ " << it->first << " ]: " << it->second << std::endl;
    }
    std::cout << std::endl;
    std::cout << "   locations [ size = " << _locations.size() << " ]" << std::endl;
    for (size_t i = 0; i < _locations.size(); i++) {
        std::cout << "   location " << _locations[i].get_path() << " {" << std::endl;
        _locations[i].print_all_directives();
    }
    std::cout << std::endl;
}
