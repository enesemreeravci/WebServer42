#include "../include/HttpParser.hpp"

/*
rawData:
"GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"

requestLine:
"GET /index.html HTTP/1.1"
*/
bool HttpParser::parseRequestLine(const std::string& rawData, HttpRequest& request)
{
    size_t end = rawData.find("\r\n");
    
    if(end == std::string::npos)
        return false;
    const std::string requestLine = rawData.substr(0, end);

    size_t first_space = requestLine.find(' ');
// find first space and validate
    if(first_space == std::string::npos || first_space == 0)
        return false;

    size_t second_space = requestLine.find(' ', first_space + 1);
    //find second space and validate
    if(second_space == std::string::npos || second_space == first_space + 1)
        return false;

        // check that the httpversion exists
    if(second_space + 1 >= requestLine.size())
        return false;
    
    const std::string method = requestLine.substr(0, first_space);
    const std::string target = requestLine.substr(first_space + 1, second_space - first_space - 1);
    const std::string version = requestLine.substr(second_space + 1);

    // store extracted values only after all basic checks succeed
    request.setMethod(method);
    request.setTarget(target);
    request.setVersion(version);
    return true;
}

bool HttpParser::parseHeader(const std::string& headerLine, HttpRequest& request)
{
    size_t colonPos = headerLine.find(':');
    // get the first : postion and validate it
    if(colonPos == 0 || colonPos == std::string::npos)
        return false;
    std::string name = headerLine.substr(0, colonPos);
    std::string value = headerLine.substr(colonPos + 1);
    while(!value.empty() && (value[0] == ' ' || value[0] == '\t'))
    {
        value.erase(0, 1);
    }
    request.addHeader(name, value);
    return true;
}

