#include "Server.hpp"
#include <iostream>

int main()
{

    Server server;
    if(!server.initializeServer(8080))
    {
        std::cerr << "Server initialiaztion failed" << std::endl;
        return 1;
    }

    server.run();
    return 0;
}
