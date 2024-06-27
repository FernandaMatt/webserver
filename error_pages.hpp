#ifndef ERROR_PAGES_HPP
# define ERROR_PAGES_HPP

//ERRORS
#define ERROR_400 "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length:511 \r\n\r\n\
<!DOCTYPE html>\
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

#define ERROR_401 "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\nContent-Length:513 \r\n\r\n\
<!DOCTYPE html>\
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

#define ERROR_403 "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length:507 \r\n\r\n\
<!DOCTYPE html>\
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

#define ERROR_404 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length:507 \r\n\r\n\
<!DOCTYPE html>\
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

#define ERROR_405 "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\nContent-Length:525 \r\n\r\n\
<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>405 Method Not Allowed</title>\
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
    <h1>405</h1>\
    <p>Method Not Allowed</p>\
    <p>by Fe, Iza e Mari</p>\
</body>\
</html>"

#define ERROR_500 "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length:531 \r\n\r\n\
<!DOCTYPE html>\
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

#define ERROR_501 "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\nContent-Length:519 \r\n\r\n\
<!DOCTYPE html>\
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

#define ERROR_502 "HTTP/1.1 502 Bad Gateway\r\nContent-Type: text/html\r\nContent-Length:511 \r\n\r\n\
<!DOCTYPE html>\
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