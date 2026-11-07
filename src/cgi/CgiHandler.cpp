#include "../../include/cgi/CgiHandler.hpp"

CgiHandler::CgiHandler()
{
    _pid = -1; // meaning no child process has been started
}


CgiHandler::~CgiHandler() {}

pid_t CgiHandler::getPid() const { return _pid; }

bool CgiHandler::reapChild()
{
    if(_pid <= 0)
        return false;

    int status;
    pid_t result = waitpid(_pid, &status, WNOHANG);
    if(result > 0)
    {
        std::cout << "Child termination status collected" << std::endl;
        _pid = -1;
        return true;
    }
    else if(result == 0)
    {
        std::cout << "Child is still running" << std::endl;
        return false;
    }
    else 
    {
        std::cout << "Error" << std::endl;
        return false;
    }
}

bool CgiHandler::startProcess()
{
    pid_t c_pid = fork();

    if(c_pid == -1)
    {
        perror("Fork");
        return false;
    }
    else if(c_pid == 0)
    {
        std::cout << "Printed form [child] process " << std::endl; 
        _exit(0);
    }
    else 
    {
        _pid = c_pid;
        std::cout << "Printed form [parent] process " << std::endl;
    }
    return true;
}
