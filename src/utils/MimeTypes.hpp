#ifndef MIMETYPES_HPP
# define MIMETYPES_HPP

# include <map>
# include <string>
# include <algorithm>
# include <iostream>

class MimeTypes {
    public:
        MimeTypes();
        std::string getType(std::string extension);
        std::string getExtension(std::string content_type);

    private:
        std::map<std::string, std::string> _mimes;
};

#endif