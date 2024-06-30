#ifndef ERROR_PAGES_HPP
# define ERROR_PAGES_HPP

#define STATUS_400 "HTTP/1.1 400 Bad Request\r\n"
#define HEADER_400 "Content-Type: text/html\r\nContent-Length:511 \r\n\r\n"
#define HTML_400 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>400 Bad Request</title>\
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
    <h1>400</h1>\
    <p>Bad Request</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

#define STATUS_401 "HTTP/1.1 401 Unauthorized\r\n"
#define HEADER_401 "Content-Type: text/html\r\nContent-Length:513 \r\n\r\n"
#define HTML_401 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>401 Unauthorized</title>\
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
    <h1>401</h1>\
    <p>Unauthorized</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"


#define STATUS_403 "HTTP/1.1 403 Forbidden\r\n"
#define HEADER_403 "Content-Type: text/html\r\nContent-Length:507 \r\n\r\n"
#define HTML_403 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>403 Forbidden</title>\
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
    <h1>403</h1>\
    <p>Forbidden</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

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

#define STATUS_501 "HTTP/1.1 501 Not Implemented\r\n"
#define HEADER_501 "Content-Type: text/html\r\nContent-Length:519 \r\n\r\n"
#define HTML_501 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>501 Not Implemented</title>\
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
    <h1>501</h1>\
    <p>Not Implemented</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

#define STATUS_502 "HTTP/1.1 502 Bad Gateway\r\n"
#define HEADER_502 "Content-Type: text/html\r\nContent-Length:511 \r\n\r\n"
#define HTML_502 "<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>502 Bad Gateway</title>\
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
    <h1>502</h1>\
    <p>Bad Gateway</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

#endif