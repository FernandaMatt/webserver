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
# include <climits>


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
        Server &operator = (const Server&obj);

        //setters
        void    set_listeners(std::string listen);
        void    set_root(std::string root);
        void    set_client_max_body_size(std::string client_max_body_size);
        void    set_timeout(std::string timeout);
        void    set_autoindex(std::string autoindex);
        void    set_server_name(std::string server_name);
        void    set_index(std::string index);
        void    set_error_page(std::string error_page);
        void    set_location(Location &location);
        void    set_host_port(const std::string &hostPort);

        //getters
        const std::vector<Listen>&  get_listeners() const;
        std::string&                get_root();
        long                        get_client_max_body_size(); //in bytes
        int                         get_timeout(); // in seconds
        std::string&                get_autoindex();
        std::vector<std::string>&   get_server_name();
        std::vector<std::string>&   get_index();
        std::map<int, std::string>& get_error_pages();
        std::string                 get_error_page_path(int error_code);
        std::vector<std::string>    get_methods();
        std::vector<Location>&      get_location();
        std::string                 get_host_port();

        void    check_port(std::string port);
        void    check_host(std::string host);
        void    check_duplicate_location(Location &location);
        void    check_duplicate_listen(std::string host, std::string port);

        void    set_default_directives();
        void    print_all_directives() const;

    private:
        std::vector<Listen>         _listeners;
        std::string                 _root;
        long                        _client_max_body_size;
        int                         _timeout;
        std::string                 _autoindex;
        std::vector<std::string>    _server_name;
        std::vector<std::string>    _index;
        std::map<int, std::string>  _error_pages;
        std::vector<Location>       _locations;
        std::string                 _host_port;
};

#endif
