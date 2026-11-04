#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "HttpRequest.hpp"
#include <string>

class HttpParser
{
    private:
        bool isValidMethod(const std::string& method) const;
        bool isValidTarget(const std::string& target) const;
        bool isValidVersion(const std::string& version) const;

        // instead of calling each of them seperatly, I put them together.
        // we can invoke only this function and it will validate every cases
        bool isValidRequest(const std::string& method, 
                            const std::string& target,
                            const std::string& version) const;

    public:
        // rawData contains the HTTP bytes received from the client
        // request the existing HTTPRequest objcet that will store parsed values
        bool parseRequestLine(const std::string& rawData, HttpRequest& request);
        bool parseHeader(const std::string& headerLine, HttpRequest& request); // processes one header
        bool parseHeaders(const std::string& rawData, HttpRequest& request); // processes multiple headers


        
};

#endif