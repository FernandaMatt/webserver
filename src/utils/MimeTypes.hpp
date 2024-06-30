#ifndef MIMETYPES_HPP
# define MIMETYPES_HPP

# include <map>
# include <string>

class MimeTypes {
    public:
        MimeTypes();
        std::string getType(std::string extension);

    private:
        std::map<std::string, std::string> _mimes;
};

#endif