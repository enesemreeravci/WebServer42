#include "../../include/cgi/CgiHandler.hpp"

int main()
{
    CgiHandler cgi;

    if(!cgi.startProcess())
    {
        std::cout << "Error occurred" << std::endl;
        return 1;
    }
    
    std::cout << "Child PID: " << cgi.getPid() << std::endl;
    std::cout << "called cgi reap: " << cgi.reapChild() << std::endl;
}