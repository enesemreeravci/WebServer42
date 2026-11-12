#include "../../include/cgi/CgiHandler.hpp"

CgiHandler::CgiHandler()
{
    _pid = -1; // meaning no child process has been started
    _inputFd = -1;
    _outputFd = -1;
    _cancelRequested = false;
}


CgiHandler::~CgiHandler()
{
    if(_inputFd != -1)
        close(_inputFd);
    if(_outputFd != -1)
        close(_outputFd);
}

pid_t CgiHandler::getPid() const { return _pid; }

int CgiHandler::getInputFd() const { return _inputFd; }

int CgiHandler::getOutputFd() const { return _outputFd; }


//if pipe calls succeed  we will have 4 new FDs
/*
cgiStdinPipe[0]	    CGI stdin read end	    Child	    read() through stdin
cgiStdinPipe[1]	    CGI stdin write end	    Parent	    write() request body
cgiStdoutPipe[0]	CGI stdout read end	    Parent	    read() CGI response
cgiStdoutPipe[1]	CGI stdout write end	Child	    write() through stdout
*/
bool CgiHandler::createPipes(int fd1[2], int fd2[2])
{
    if(pipe(fd1) == -1)
    {
        std::cerr << "pipe creation failed" << std::endl;
        return false;
    }

    if(pipe(fd2) == -1)
    {
        std::cerr << "pipe creation failed" << std::endl;
        close(fd1[0]);
        close(fd1[1]);
        return false;
    }
    return true;
}


ReapResult CgiHandler::reapChild()
{
    if(_pid <= 0)
        return REAP_ERROR;

    int status;
    pid_t result = waitpid(_pid, &status, WNOHANG);
    if(result > 0)
    {
        std::cout << "Child termination status collected" << std::endl;
        _pid = -1;
        if(WIFEXITED(status) == true)
            std::cout << "Child exit code: " << WEXITSTATUS(status) << std::endl;
        else if(WIFSIGNALED(status))
            std::cout << "Terminated by signal" << std::endl;

        return REAP_FINISHED;
    }
    else if(result == 0)
    {
        std::cout << "Child is still running" << std::endl;
        return REAP_RUNNING;
    }
    else 
    {
        std::cout << "Error" << std::endl;
        return REAP_ERROR;
    }
}

bool CgiHandler::terminateChild()
{
    if(_pid <= 0)
        return false;

    if(kill(_pid , SIGTERM) == -1) // standart graceful shutdown request
        return false;
    _cancelRequested = true;
    return true;
}

bool CgiHandler::isCancelRequested() const
{
    if(_cancelRequested == true)
        return true;
    return false;
}

bool CgiHandler::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL); // get flags
    if(flags == -1)
        return false;
    int result = fcntl(fd, F_SETFL, flags | O_NONBLOCK); // old flags + nonblock
    if(result == -1)
        return false;
    return true;
}

bool CgiHandler::isRunning() const
{
    if(_pid > 0)
        return true;    
    return false;
}

bool CgiHandler::startProcess(const std::string& executablePath)
{
    
    int fd1[2];
    int fd2[2];
    
    if(createPipes(fd1, fd2) == false)
        return false;
    pid_t c_pid = fork();
    
    if(c_pid == -1)
    {
        perror("Fork");
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[1]);
        close(fd2[0]);
        return false;
    }
    else if(c_pid == 0) // child
    {
        close(fd1[1]); // because child doesnt write to pipe 1
        close(fd2[0]);  // because child doesnt read from pipe 2
      
        if(dup2(fd1[0], STDIN_FILENO) == -1)
        {
            perror("dup2 failed");
            close(fd1[0]);
            close(fd2[1]);
            _exit(1);
        }
        if(dup2(fd2[1], STDOUT_FILENO) == -1)
        {
            perror("dup2 failed");
            close(fd1[0]);
            close(fd2[1]);
            _exit(1);
        }
        close(fd1[0]);
        close(fd2[1]);

        char *argv[] = { const_cast<char*>(executablePath.c_str()), 
        NULL };

        char *envp[] = { NULL };

        if(execve(executablePath.c_str(), argv, envp) == -1)
        {
            perror("execve");
            _exit(1);
        }


    }
    else // parent c_pid > 0 
    {
        _pid = c_pid;
        close(fd1[0]);  // because parent doesnt read from pipe 1
        close(fd2[1]);  // because parent doesnt write from pipe 2


        _inputFd = fd1[1]; // parent writes the request body
        _outputFd = fd2[0]; // parents read cgi output
        if (!setNonBlocking(_inputFd) || !setNonBlocking(_outputFd))
        {
            close(_inputFd);
            close(_outputFd);
            _inputFd = -1;
            _outputFd = -1;
            return false;
        }
        std::cout << "Printed form [parent] process " << std::endl;
    }
    return true;
}
