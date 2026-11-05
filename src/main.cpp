#include "Server.hpp"
#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include <iostream>

int main()

{

    // testing my parsers
    HttpRequest request;
    HttpParser parser;


    const std::string rawData =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Accept: text/html\r\n"
        "Connection: close\r\n"
        "\r\n";

    const std::string rawData2 =
    "GET / HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "InvalidHeaderWithoutColon\r\n"
    "Connection: close\r\n"
    "\r\n";
    // std::string headerLine = "Host:    localhost:8080";
    
    // if(parser.parseRequestLine(rawData, request))
    // {
    //     std::cout << "Method: " << request.getMethod() << std::endl;
    //     std::cout << "Target: " << request.getTarget() << std::endl;
    //     std::cout << "Version: " << request.getVersion() << std::endl;
    // }

    
    std::map<std::string, std::string>::const_iterator it;
    if(parser.parseHeaders(rawData2, request))
    {
        const std::map<std::string, std::string>& headers = request.getHeaders();
        for(it = headers.begin(); it != headers.end(); ++it)
        {
            std::cout << "Key: " << it->first << ", " << "Value: " << it->second << std::endl; 
        }

    }
    else
    {
        std::cout << "Failed to parse  headers" << std::endl;
    }
    
    Server server;
    if(!server.initializeServer(8080))
    {
        std::cerr << "Server initialiaztion failed" << std::endl;
        return 1;
    }

    server.run();
    return 0;
}
