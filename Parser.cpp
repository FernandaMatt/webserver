#include "Parser.hpp"

Parser::Parser(std::string const &configFile) : _file(configFile) {
    parsing(_file);
    reformat_config(_content);
    split_servers(_content);
    for (size_t i = 0; i < _server_block.size(); ++i) {
        parse_directives(_server_block[i]);
    }
    check_duplicate_server_names();
}

Parser::~Parser() {}

//getters
size_t  Parser::get_nbr_servers() {
    return _server_block.size();
}

std::vector<Server> Parser::get_servers() {
    return _servers;
}

//check file, passes the content to string and remove comments
void    Parser::parsing(std::string const &config_file) {
    std::ifstream file(config_file);
    std::string line;

    if (access(_file.c_str(), R_OK) == -1) {
        throw std::runtime_error("Error in access()");
    }
    if (!file || !file.is_open()) {
        throw std::runtime_error("Error in open");
    }
    while (std::getline(file, line)) {
        remove_comment(line);
        _content += line;
    }
    if (_content.empty()) {
        throw std::runtime_error("Invalid configuration: file is empty");
    }
}

//find '#' and delete the rest of the line
void    Parser::remove_comment(std::string &line) {
    size_t comment = line.find('#');
    if (comment != std::string::npos) {
        line.erase(comment);
    }
}

//reformat the file
void    Parser::reformat_config(std::string &content) {
    std::string aux;
    bool    was_space = false;

    //split the words, braces and semicolon in lines
    for (size_t i = 0; i < content.size(); ++i) {
        if (std::isspace(content[i])) {
            if (!was_space) {
                aux += '\n';
                was_space = true;
            }
        } else {
            if (content[i] == '{' || content[i] == '}' || content[i] == ';') {
                if (!was_space)
                aux += '\n';
                aux += content[i];
                aux += '\n';
                was_space = true;
            } else {
                aux += content[i];
                was_space = false;
            }
        }
    }
    content.clear();
    // format the file to be read
    for (size_t i = 0; i < aux.size(); i++) {
        if (aux[i] == ';')
            content += ";\n";
        else if (aux[i] == '{' || aux[i] == '}'){
            content += aux[i];
            content += '\n';
        }
        else if (aux[i] == '\n') {
            if (aux[i + 1] && aux[i + 1] == ';')
                continue;
            else if (aux[i - 1] && aux[i - 1] != ';' &&  aux[i - 1] != '{' && aux[i - 1] != '}')
                content += ' ';
        }
        else
            content += aux[i];
    }
}


//split each server block
void    Parser::split_servers(std::string const &content) {
    size_t              start = 0;
    size_t              end = 0;
    int                 brace = 0;

    for (size_t pos = 0; pos < content.size(); pos++) {
        while (pos < content.size() && std::isspace(content[pos])) pos++;
        //start a server block
        if (content.compare(pos, 8, "server {") == 0 && brace == 0) {
            start = pos;
            pos += 8;
            brace++;

            while (pos < content.length() && std::isspace(content[pos])) pos++;
            //find the end of the server and count braces
            while (pos < content.length() && brace > 0) {
                if (content[pos] == '{') brace++;
                else if (content[pos] == '}') brace--;
                pos++;
            }
            if (brace != 0)
               throw std::runtime_error("Error in config file: unclosed brace");
            end = pos - 1;
             _server_block.push_back(content.substr(start, end - start + 1));
            continue;
        } else
            throw std::runtime_error("Error in config file: invalid directive");
    }
}

void    Parser::set_server_directives(std::string &key, std::string &value, Server &server) {
    size_t pos_semicolon = value.find(';');
    if (pos_semicolon == std::string::npos)
        throw std::runtime_error("Error in config file: semicolon missing");
    value.erase(pos_semicolon);

    if (key == "listen")
        server.set_listeners(value);
    else if (key == "root")
        server.set_root(value);
    else if (key == "client_max_body_size")
        server.set_client_max_body_size(value);
    else if (key == "server_name")
        server.set_server_name(value);
    else if (key == "index")
        server.set_index(value);
    else if (key == "autoindex")
        server.set_autoindex(value);
    else if (key == "error_page")
        server.set_error_page(value);

    key.clear();
    value.clear();
}

void    Parser::set_location_directives(std::string &key, std::string &value, Location &location) {
    size_t pos_semicolon = value.find(';');
    if (pos_semicolon == std::string::npos)
        throw std::runtime_error("Error in config file: semicolon missing");
    value.erase(pos_semicolon);

    if (key == "root")
        location.set_root(value);
    else if (key == "alias")
        location.set_alias(value);
    else if (key == "client_max_body_size")
        location.set_client_max_body_size(value);
    else if (key == "index")
        location.set_index(value);
    else if (key == "autoindex")
        location.set_autoindex(value);
    else if (key == "error_page")
        location.set_error_page(value);
    else if (key == "allow_methods")
        location.set_methods(value);
    else if (key == "upload_path")
        location.set_upload_path(value);
    else if (key == "cgi_path")
        location.set_cgi_path(value);
    else if (key == "cgi_ext")
        location.set_cgi_ext(value);

    key.clear();
    value.clear();
}

//create server, check and set all directives
void    Parser::parse_directives(std::string const &server_block) {
    Server  server;
    std::stringstream ss(server_block);
    std::string line;
    bool    in_server = false;

    while (std::getline(ss, line)) {
        if (line.empty())
            continue;
        else if (line.compare(0, 8, "server {") == 0 && !in_server){
            in_server = true;
            continue;
        }
        //compare if exists another server directive inside an server block
        else if (line.compare(0, 8, "server {") == 0 && in_server)
            throw std::runtime_error("Error in config file: invalid directive inside a server scope");
        else if (line[0] == '}') break;

        //split key and value for set_directives
        size_t pos_space = line.find(' ');
        if (pos_space == std::string::npos)
            throw std::runtime_error("Error in config file: " + line);

        std::string key = line.substr(0, pos_space);
        std::string value = line.substr(pos_space + 1);

        //check and set valid directives
        if (key == "location"){
            Location location;
            std::string location_block = line + '\n';
            while (std::getline(ss, line)) {
                location_block += line + '\n';
                if (line[0] == '}') break;
            }
            parse_location(location_block, location);
            server.set_location(location);
            location_block.clear();
            continue;
        }
        else if (key == "listen" || key == "server_name" || key == "index" ||
                    key == "autoindex" || key == "root" ||
                    key == "client_max_body_size" || key == "error_page")
            set_server_directives(key, value, server);
        //invalid directive or other error
        else
            throw std::runtime_error("Error in config file: " + line);
    }

    server.set_default_directives();
    // server.set_sock_fd();
    _servers.push_back(server);
}

Location    &Parser::parse_location(std::string &location_str, Location &location) {
    std::stringstream ss(location_str);
    std::string line;
    bool    in_path = true;

    while (std::getline(ss, line)) {
        if (line.empty())
            continue;
        else if (line[0] == '}') break;

        //split key and value for set_directives
        size_t pos_space = line.find(' ');
        if (pos_space == std::string::npos)
            throw std::runtime_error("Error in location config file: " + line);

        std::string key = line.substr(0, pos_space);
        std::string value = line.substr(pos_space + 1);

        //set path location
        if (in_path == true && key == "location") {
            in_path == false;

            pos_space = value.find(' ');
            if (pos_space == std::string::npos)
                throw std::runtime_error("Error in location config file: Invalid number of arguments in location directive" + line);
            std::string path = value.substr(0, pos_space);
            std::string after_path = value.substr(pos_space + 1);
            if (after_path != "{")
                throw std::runtime_error("Error in location config file: " + line);
            location.set_path(path);
        }
        else if (key == "root" || key == "alias" || key == "index" || key == "autoindex" ||
                    key == "allow_methods" || key == "client_max_body_size" ||
                    key == "error_page" || key == "cgi_path" || key == "cgi_ext" || key == "upload_path")
            set_location_directives(key, value, location);
        //invalid directive or other error
        else
            throw std::runtime_error("Error in config file: " + line);
    }

    return location;
}

void    Parser::check_duplicate_server_names() {
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
        std::vector<std::string>& current_server_names = it->get_server_name();
        for (std::vector<std::string>::iterator it_name = current_server_names.begin(); it_name != current_server_names.end(); it_name++) {
            std::string current_name = *it_name;
            for (std::vector<Server>::iterator next_it = it + 1; next_it != _servers.end(); next_it++) {
                std::vector<std::string>& next_server_names = next_it->get_server_name();
                std::vector<std::string>::iterator name_found = std::find(next_server_names.begin(), next_server_names.end(), current_name);
                if (name_found != next_server_names.end()) {
                    std::vector<Listen> it_listeners = it->get_listeners();
                    std::vector<Listen> next_listeners = next_it->get_listeners();
                    bool found = false;
                    for (std::vector<Listen>::iterator it_listen = it_listeners.begin(); it_listen != it_listeners.end(); it_listen++) {
                        for(std::vector<Listen>::iterator next_listen = next_listeners.begin(); next_listen != next_listeners.end(); next_listen++) {
                            if (it_listen->host == next_listen->host && it_listen->port == next_listen->port) {
                                if (found == false)
                                    next_server_names.erase(name_found);
                                found = true;
                                Logger::log(LOG_WARNING, "conflicting server name " + current_name + " on " + it_listen->host + ":" + it_listen->port + ", ignored");
                                break ;
                            }
                        }
                    }
                }
            }
        }
    }
}

void    Parser::print_servers_directives() {
    for (std::vector<Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
		it->print_all_directives();
}
