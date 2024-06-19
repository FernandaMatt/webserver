#ifndef ERROR_PAGES_HPP
# define ERROR_PAGES_HPP

# define STATUS_404 "HTTP/1.1 404 Not Found\r\n"
# define HEADER_404 "Content-Type: text/html\r\nContent-Length:507 \r\n\r\n"
# define HTML_404 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>404 Not Found</title>\
    <style>\
        body {\
            font-family: Arial, sans-serif;\
            text-align: center;\
            margin-top: 50px;\
        }\
        h1 {\
            font-size: 100px;\
        }\
        p {\
            font-size: 20px;\
        }\
    </style>\
</head>\
<body>\
    <h1>404</h1>\
    <p>Not Found</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

# define STATUS_405 "HTTP/1.1 405 Method Not Allowed\r\n"
#define HEADER_405 "Content-Type: text/html\r\nContent-Length:525 \r\n\r\n"
# define HTML_405 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>404 Method Not Allowed</title>\
    <style>\
        body {\
            font-family: Arial, sans-serif;\
            text-align: center;\
            margin-top: 50px;\
        }\
        h1 {\
            font-size: 100px;\
        }\
        p {\
            font-size: 20px;\
        }\
    </style>\
</head>\
<body>\
    <h1>404</h1>\
    <p>Method Not Allowed</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

# define STATUS_413 "HTTP/1.1 413 Request Entity Too Large\r\n"
# define HEADER_413 "Content-Type: text/html\r\nContent-Length:537 \r\n\r\n"
# define HTML_413 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>404 Request Entity Too Large</title>\
    <style>\
        body {\
            font-family: Arial, sans-serif;\
            text-align: center;\
            margin-top: 50px;\
        }\
        h1 {\
            font-size: 100px;\
        }\
        p {\
            font-size: 20px;\
        }\
    </style>\
</head>\
<body>\
    <h1>404</h1>\
    <p>Request Entity Too Large</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

# define STATUS_500 "HTTP/1.1 500 Internal Server Error\r\n"
# define HEADER_500 "Content-Type: text/html\r\nContent-Length: 531\r\n\r\n"
# define HTML_500 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>500 Internal Server Error</title>\
    <style>\
        body {\
            font-family: Arial, sans-serif;\
            text-align: center;\
            margin-top: 50px;\
        }\
        h1 {\
            font-size: 100px;\
        }\
        p {\
            font-size: 20px;\
        }\
    </style>\
</head>\
<body>\
    <h1>500</h1>\
    <p>Internal Server Error</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

#endif
