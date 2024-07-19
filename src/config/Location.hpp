#ifndef LOCATION_HPP
# define LOCATION_HPP
# include "../utils/defines.hpp"
# include <iostream>
# include <map>
# include <vector>
# include <sys/stat.h>
# include <climits>


class Location {
    public:
        Location();
        ~Location();
        Location    &operator = (const Location& obj);

        //setters
        void    set_path(std::string path);
        void    set_root(std::string root);
        void    set_alias(std::string alias);
        void    set_client_max_body_size(std::string client_max_body_size);
        void    set_timeout(std::string timeout);
        void    set_autoindex(std::string autoindex);
        void    set_index(std::string index);
        void    set_error_page(std::string error_pages);
        void    set_methods(std::string methods);
        void    set_upload_path(std::string upload_path);
        void    set_cgi_path(std::string cgi_path);
        void    set_cgi_ext(std::string cgi_ext);

        //getters
    	std::string                 get_path() const;
        std::string&                get_root();
        std::string&                get_alias();
        long                        get_client_max_body_size();
        int                         get_timeout(); // in seconds
        std::string&                get_autoindex();
        std::vector<std::string>&   get_index();
        std::map<int, std::string>& get_error_pages();
        std::string                 get_error_page_path(int error_code);
        std::vector<std::string>&   get_methods();
        std::string&                get_upload_path();
        std::string&                get_cgi_path();
        std::string&                get_cgi_ext();

        std::string                 search_index_file(std::string path);

        void    print_all_directives() const;

    private:
        std::string                 _path;
        std::string                 _root;
        std::string                 _alias;
        long                        _client_max_body_size;
        int                         _timeout;
        std::string                 _autoindex;
        std::vector<std::string>    _index;
        std::vector<std::string>    _methods;
        std::map<int, std::string>  _error_pages;
        std::string                 _upload_path;
        std::string                 _cgi_path;
        std::string                 _cgi_ext;
};

#endif
