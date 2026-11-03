#include "Server.hpp"
#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include <iostream>

int main()

{

    // testing my parsers
    HttpRequest request;
    HttpParser parser;


    const std::string rawData = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string headerLine = "Host:    localhost:8080";
    
    if(parser.parseRequestLine(rawData, request))
    {
        std::cout << "Method: " << request.getMethod() << std::endl;
        std::cout << "Target: " << request.getTarget() << std::endl;
        std::cout << "Version: " << request.getVersion() << std::endl;
    }

    const std::map<std::string, std::string>& headers = request.getHeaders();

    std::map<std::string, std::string>::const_iterator it;
    if(parser.parseHeader(headerLine, request))
    {
        request.getHeaders();
        it = headers.find("Host");
        if(it != headers.end())
        {
            std::cout << "Key: " << it->first << std::endl;
            std::cout << "Value: " << it->second << std::endl; 
        }
    }
    
    // Server server;
    // if(!server.initializeServer(8080))
    // {
    //     std::cerr << "Server initialiaztion failed" << std::endl;
    //     return 1;
    // }

    // server.run();
    return 0;
}
