#ifndef LOCATION_HPP
# define LOCATION_HPP
# include <iostream>
#include <map>
#include <vector>

class Location {
    public:
        Location();
        ~Location();

        //setters
        void    set_path(std::string path);
        void    set_root(std::string root);
        void    set_alias(std::string alias);
        void    set_client_max_body_size(std::string client_max_body_size);
        void    set_autoindex(std::string autoindex);
        void    set_index(std::string index);
        void    set_error_page(std::string error_pages);
        void    set_methods(std::string methods);

        //getters
    	std::string                 get_path();
        std::string                 get_root();
        std::string                 get_alias();
        unsigned long               get_client_max_body_size();
        std::string                 get_autoindex();
        std::vector<std::string>    get_index();
        std::map<int, std::string>  get_error_pages();
        std::string                 get_error_page_path(int error_code);
        std::vector<std::string>    get_methods();


        void    check_directives();
        void    print_all_directives();

    private:
        std::string                 _path;
        std::string                 _root;
        std::string                 _alias;
        unsigned long               _client_max_body_size;
        std::string                 _autoindex;
        std::vector<std::string>    _index;
        std::vector<std::string>    _methods;
        std::map<int, std::string>  _error_pages;
        //add cgi
};

#endif