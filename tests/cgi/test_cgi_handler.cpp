#include "../../include/cgi/CgiHandler.hpp"
#include <iostream>

int main()
{
    char buffer[1024];

    CgiHandler obj;
    if(!obj.startProcess("/bin/echo"))
    {
        std::cerr << "Error: starting process" << std::endl;
        return 1;
    }
    ssize_t bytesRead = read(
        obj.getOutputFd(),
        buffer,
        sizeof(buffer)
    );
    
    if(bytesRead == -1)
    {
        perror("read failed");
        return 2;
    }
    
    if(bytesRead == 0)
    {
        std::cout << "EOF" << std::endl;
    }
    if(bytesRead > 0)
    {
        std::cout << "Bytes Received: " << bytesRead << std::endl;
    }
    std::cout << "Child's PID: " << obj.getPid() << std::endl;
    ReapResult result = obj.reapChild();
    if (result == REAP_RUNNING)
    {
        std::cout << "Child is still running" << std::endl;
    }
    else if(result == REAP_FINISHED)
    {
        std::cout << "Child was reaped" << std::endl;
    }
    else if(result == REAP_ERROR)
    {
        std::cout << "Error while reaping child" << std::endl;
    }
    return 0;
}