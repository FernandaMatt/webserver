#include "ResponseChecker.hpp"

ResponseChecker::ResponseChecker() {}

ResponseChecker::ResponseChecker(std::string response) : _response(response) {}

ResponseChecker::~ResponseChecker() {}

int ResponseChecker::getResponseStatusCode() {
    if (!isStatusLineValid())
        return 500;
    
    return true;
}

bool ResponseChecker::isStatusLineValid() {
    size_t pos = _response.find("HTTP/1.1");
    if (pos == std::string::npos || pos < 0)
        return false;
    return true;
}