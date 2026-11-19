#include "../../include/cgi/CgiHandler.hpp"
#include "../../include/utils/Utils.hpp"

CgiHandler::CgiHandler()
{
    _pid = -1; // meaning no child process has been started
    _inputFd = -1;
    _outputFd = -1;
    _cancelRequested = false;
    _bytesWritten = 0;
    _startTime = 0;
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

void CgiHandler::setInputBuffer(const std::string& body)
{
    _inputBuffer = body;
    _bytesWritten = 0;
}

const std::string& CgiHandler::getOutputBuffer() const
{
    return _outputBuffer;
}

bool CgiHandler::isTimedOut(int timeoutSeconds) const
{
    if(_startTime == 0)
        return false;
    return (std::time(NULL) - _startTime) >= timeoutSeconds;
}

bool CgiHandler::writeToStdin()
{
    if(_inputFd == -1)
        return true;
    
    if(_bytesWritten >= _inputBuffer.size())
    {
        close(_inputFd);
        _inputFd = -1;
        return true;
    }

    ssize_t remaining = _inputBuffer.size() - _bytesWritten;
    ssize_t written = write(_inputFd, _inputBuffer.c_str() + _bytesWritten, remaining);
    if(written > 0)
    {
        // static_cast used to prevent complie error, later I check wheter it is equal to -1 or not
        _bytesWritten += static_cast<size_t>(written);
        if(_bytesWritten >= _inputBuffer.size())
        {
            close(_inputFd);
            _inputFd = -1;
            return true;
        }
        return false;
    }
    if((written == -1) && (errno == EAGAIN || errno == EWOULDBLOCK))
        return false;
    
    close(_inputFd);
    _inputFd = -1;
    return true;
}

bool CgiHandler::readFromStdout()
{
    if(_outputFd == -1)
        return true;
    
    char buffer[4096];
    ssize_t bytesRead = read(_outputFd, buffer, sizeof(buffer));
    if(bytesRead > 0)
    {
        _outputBuffer.append(buffer, static_cast<size_t>(bytesRead));
        return false;
    }

    if((bytesRead == -1) && (errno == EAGAIN || errno == EWOULDBLOCK))
        return false;
    
    // eof or read error
    close(_outputFd);
    _outputFd = -1;
    return true;
}


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

bool CgiHandler::isRunning() const
{
    if(_pid > 0)
        return true;    
    return false;
}

bool CgiHandler::startProcess(const std::string& scriptPath,
                              const std::string& interpreterPath,
                              const std::vector<std::string>& envVars)
{
    int fd1[2];
    int fd2[2];

    if (!createPipes(fd1, fd2))
        return false;

    pid_t c_pid = fork();
    if (c_pid == -1)
    {
        perror("fork");
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);
        return false;
    }
    else if (c_pid == 0) // CHILD PROCESS
    {
        close(fd1[1]); 
        close(fd2[0]);

        if (dup2(fd1[0], STDIN_FILENO) == -1 || dup2(fd2[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(fd1[0]);
            close(fd2[1]);
            _exit(1);
        }
        close(fd1[0]);
        close(fd2[1]);

        std::string scriptFile = scriptPath;
        size_t lastSlash = scriptPath.find_last_of('/');
        if (lastSlash != std::string::npos)
        {
            std::string dir = scriptPath.substr(0, lastSlash);
            scriptFile = scriptPath.substr(lastSlash + 1);
            if (!dir.empty() && chdir(dir.c_str()) == -1)
            {
                perror("chdir");
                _exit(1);
            }
        }

        std::vector<char*> envp;
        for (size_t i = 0; i < envVars.size(); ++i)
            envp.push_back(const_cast<char*>(envVars[i].c_str()));
        envp.push_back(NULL);

        std::vector<char*> argv;
        std::string execTarget;
        if (!interpreterPath.empty())
        {
            execTarget = interpreterPath;
            argv.push_back(const_cast<char*>(interpreterPath.c_str()));
            argv.push_back(const_cast<char*>(scriptFile.c_str()));
        }
        else
        {
            execTarget = scriptFile;
            argv.push_back(const_cast<char*>(scriptFile.c_str()));
        }
        argv.push_back(NULL);

        execve(execTarget.c_str(), &argv[0], &envp[0]);
        perror("execve");
        _exit(1);
    }
    else // PARENT PROCESS
    {
        _pid = c_pid;
        _startTime = std::time(NULL);
        _bytesWritten = 0;

        close(fd1[0]); 
        close(fd2[1]); 

        _outputFd = fd2[0];
        if (!Utils::setNonBlocking(_outputFd))
        {
            close(fd1[1]);
            close(_outputFd);
            _outputFd = -1;
            return false;
        }

        if (_inputBuffer.empty())
        {
            close(fd1[1]);
            _inputFd = -1;
        }
        else
        {
            _inputFd = fd1[1];
            if (!Utils::setNonBlocking(_inputFd))
            {
                close(_inputFd);
                close(_outputFd);
                _inputFd = -1;
                _outputFd = -1;
                return false;
            }
        }
    }
    return true;
}