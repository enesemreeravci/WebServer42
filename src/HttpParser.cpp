#include "../include/HttpParser.hpp"

/*
rawData:
"GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"

requestLine:
"GET /index.html HTTP/1.1"
*/

bool HttpParser::isValidMethod(const std::string& method) const
{
    return (method == "GET" || method == "POST" || method == "DELETE");
}

bool HttpParser::isValidTarget(const std::string& target) const
{
    if(target.empty())
        return false;
    
    if(target[0] != '/') // target starts always with '/'
        return false;
    
        // no need for this, remove later!
        // check path traversel
    if(target.find("..") != std::string::npos)
        return false;

    for(size_t i = 0; i < target.size(); i++)
    {
        char c = target[i];

        if(c <= 32 || c == 127)
            return false;

        // check characteres that must not be in target
        if(c == '"' || c == '\'' || c == '<' || c == '>')
            return false;
    }

    // this can be remove later, i just wanted to limit the target lentgh
    if(target.length() > 2048)
        return false;
    
    return true;
}

bool HttpParser::isValidVersion(const std::string& version) const
{
    return (version == "HTTP/1.1" || version == "HTTP/1.0");
}

bool HttpParser::isValidRequest(const std::string& method,
                                const std::string& target,
                                const std::string& version) const
{
    return (isValidMethod(method) && isValidTarget(target) && isValidVersion(version));   
}

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


    if(!isValidRequest(method, target, version))
        return false;
    
    request.setMethod(method);    
    // store extracted values only after all basic checks succeed
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

bool HttpParser::parseHeaders(const std::string& rawData, HttpRequest& request)
{
    size_t requestLineEnd = rawData.find("\r\n");
    if(requestLineEnd == std::string::npos)
        return false;
    // requestLine: "GET /index.html HTTP/1.1"
    // we kinda skipping the request line and getting the position of header
    size_t headersStart = requestLineEnd + 2; // it points to H in "Host"

    size_t headersEnd = rawData.find("\r\n\r\n"); 
    if(headersEnd == std::string::npos)
    return false;
    
    while (headersStart < headersEnd)
    {
        size_t nextHeaderEnd = rawData.find("\r\n", headersStart); 
        // searched from H to end of that line
        // so it points to the \r immediately after localhost
            
        if(nextHeaderEnd == std::string::npos)
            return false;
        if(nextHeaderEnd > headersEnd) 
            return false;
        const std::string headerLine = rawData.substr(headersStart, nextHeaderEnd - headersStart);
        
        if(!parseHeader(headerLine, request))
            return false;
        headersStart = nextHeaderEnd + 2;
    }
    return true;
    
    
}                       