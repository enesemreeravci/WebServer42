#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>

class CgiHandler
{
    private:
        pid_t _pid;  // stores the child process id after a succesfull fork()

        int _inputFd; // parent's write end for sending the htpp request body to the cgi child
        int _outputFd; // parent's read end for receiving the cgi childs' output

    public:
        CgiHandler();
        ~CgiHandler();


        bool createPipes(int fd1[2], int fd2[2]);
        bool startProcess(const std::string& executablePath);
        bool reapChild();


        // getters
        pid_t getPid() const;
        int getInputFd() const;
        int getOutputFd() const;

};

#endif