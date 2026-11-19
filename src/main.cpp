#include "../include/network/Server.hpp"
#include "../include/http/HttpRequest.hpp"
#include "../include/http/HttpParser.hpp"
#include <iostream>

int main()

{
 
    Server server;
    if(!server.setupListeninSocket(8080))
    {
        std::cerr << "Server initialiaztion failed" << std::endl;
        return 1;
    }
    
    std::cout << "Server listenin on port 8080" << std::endl;

    server.run();
    return 0;
}
