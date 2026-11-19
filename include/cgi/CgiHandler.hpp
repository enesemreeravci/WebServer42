#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ctime>
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <string>
#include <vector>

enum ReapResult
{
    REAP_ERROR,
    REAP_RUNNING,
    REAP_FINISHED
};

class CgiHandler
{
    private:
        pid_t _pid;
        int _inputFd;
        int _outputFd;
        bool _cancelRequested;

        std::string _inputBuffer;
        size_t _bytesWritten;
        std::string _outputBuffer;
        time_t _startTime;

        CgiHandler(const CgiHandler& other);
        CgiHandler& operator=(const CgiHandler& other);

    public:
        CgiHandler();
        ~CgiHandler();

        bool createPipes(int fd1[2], int fd2[2]);
        bool startProcess(const std::string& scriptPath,
                          const std::string& interpreterPath,
                          const std::vector<std::string>& envVars);
        ReapResult reapChild();
        bool terminateChild();
        bool isRunning() const;
        bool isCancelRequested() const;
        bool writeToStdin();
        bool readFromStdout();

        // getters and setters
        pid_t getPid() const;
        int getInputFd() const;
        int getOutputFd() const;
        void setInputBuffer(const std::string& body);
        const std::string& getOutputBuffer() const;
        bool isTimedOut(int timeoutSeconds) const;
};

#endif