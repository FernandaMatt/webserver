#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP
# include "Logger.hpp"
# include "Location.hpp"
# include <string>
# include <iostream>
# include <map>
# include <vector>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <unistd.h>
# include <stdexcept>
# include <cstring>
# include <errno.h>

class Location;
class Listener;

class Server {
    public:
        Server();
        ~Server();
        
        //setters
        void    set_listen(std::string listen);
        void    set_port(std::string port);
        void    set_host(std::string host);
        void    set_root(std::string root);
        void    set_client_max_body_size(std::string client_max_body_size);
        void    set_autoindex(std::string autoindex);
        void    set_server_name(std::string server_name);
        void    set_index(std::string index);
        void    set_error_page(std::string error_page);
        void    set_methods(std::string methods);
        void    set_location(Location &location);
        void    set_listener();

        //getters
        std::string                 get_port();
        std::string                 get_host();
        std::string                 get_root();
        unsigned long               get_client_max_body_size(); //in bytes
        std::string                 get_autoindex();
        std::vector<std::string>    get_server_name();
        std::vector<std::string>    get_index();
        std::map<int, std::string>  get_error_pages();
        std::string                 get_error_page_path(int error_code);
        std::vector<std::string>    get_methods();
        std::vector<Location>       get_location();
        int                         get_sock_fd();

        //void    init_error_pages();
        bool    listen(void) const;
        void    print_all_directives();

    private:
        std::string                 _port;
        std::string                 _host;
        std::string                 _root;
        unsigned long               _client_max_body_size;
        std::string                 _autoindex;
        std::vector<std::string>    _server_name;
        std::vector<std::string>    _index;
        std::map<int, std::string>  _error_pages;
        std::vector<std::string>    _methods;
        std::vector<Location>       _locations;
        
        int                         _sock_fd;
        struct sockaddr_storage     _addr_info;
};

//set default error pages
//

#endif