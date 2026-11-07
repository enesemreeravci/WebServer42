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


    public:
        CgiHandler();
        ~CgiHandler();

        bool startProcess();
        bool reapChild();
        pid_t getPid() const;
};

#endif