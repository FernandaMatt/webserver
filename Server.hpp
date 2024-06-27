#ifndef SERVER_HPP
# define SERVER_HPP
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
# include <sstream>

class Location;
class Listener;

struct Listen {
    std::string host;
    std::string port;
};
class Server {
    public:
        Server();
        ~Server();

        //setters
        void    set_listeners(std::string listen);
        void    set_root(std::string root);
        void    set_client_max_body_size(std::string client_max_body_size);
        void    set_autoindex(std::string autoindex);
        void    set_server_name(std::string server_name);
        void    set_index(std::string index);
        void    set_error_page(std::string error_page);
        void    set_location(Location &location);
        void    set_sock_fd(); //create and bind

        //getters
        const std::vector<Listen>&  get_listeners() const;
        std::string&                get_root();
        long                        get_client_max_body_size(); //in bytes
        std::string&                get_autoindex();
        std::vector<std::string>&   get_server_name();
        std::vector<std::string>&   get_index();
        std::map<int, std::string>& get_error_pages();
        std::string                 get_error_page_path(int error_code);
        std::vector<Location>&      get_location();
        const std::vector<int>&     get_sock_fd() const;

        void    check_port(std::string port);
        void    check_host(std::string host);
        void    check_duplicate_location(Location &location);
        void    check_duplicate_listen(std::string host, std::string port);

        void    set_default_directives();
        void    print_all_directives();

    private:
        std::vector<Listen>         _listeners;
        std::string                 _root;
        long                        _client_max_body_size;
        std::string                 _autoindex;
        std::vector<std::string>    _server_name;
        std::vector<std::string>    _index;
        std::map<int, std::string>  _error_pages;
        std::vector<Location>       _locations;

        std::vector<int>            _sock_fd;
};

#endif
