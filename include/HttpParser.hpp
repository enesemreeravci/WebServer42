#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "HttpRequest.hpp"
#include <string>

class HttpParser
{
    private:

    public:
        // rawData contains the HTTP bytes received from the client
        // request the existing HTTPRequest objcet that will store parsed values
        bool parseRequestLine(const std::string& rawData, HttpRequest& request);
        bool parseHeader(const std::string& headerLine, HttpRequest& request);

};

#endif