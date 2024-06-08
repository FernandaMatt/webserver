#ifndef PARSER_HPP
# define PARSER_HPP
# include "Logger.hpp"
# include "Server.hpp"
# include <iostream>
# include <string>
# include <fstream>
# include <sstream>
# include <unistd.h>
# include <vector>
# include <stdexcept>

class Server;

class Parser {
    public:
        Parser(std::string const &config_file);
        ~Parser();
        
        //getters
        size_t              get_nbr_servers();
        std::vector<Server> get_servers();
        
    private:
        std::string const           _file;
        std::string                 _content;
        std::vector<std::string>    _server_block;
        std::vector<Server>         _servers;

        //methods
        void        parsing(std::string const &config_file);
        void        reformat_config(std::string &content);
        void        remove_comment(std::string &line);
        void        split_servers(std::string const &content);
        void        parse_directives(std::string const &server_block);
        Location    &parse_location(std::string &location_block, Location &location);
        void        set_server_directives(std::string &key, std::string &value, Server &server);
        void        set_location_directives(std::string &key, std::string &value, Location &location);
};

#endif