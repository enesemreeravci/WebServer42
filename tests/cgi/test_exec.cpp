#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/wait.h>
#include <stdio.h>

int main()
{

    char *argv[] = {
    const_cast<char*>("/bin/echo"),
    const_cast<char*>("Hello from CGI"),
    NULL
    };

    char *envp[] = { NULL };
    pid_t c_pid = fork();
    if(c_pid == -1)
    {
        return 1;
    }
    if(c_pid == 0)
    {
        if(execve("/bin/echo", argv, envp) == -1)
        {
            perror("execve failed");
            _exit(1);
        } 
    }
    else if(c_pid > 0)
    {
        int status;
        int res = waitpid(c_pid, &status, 0);
        if(res == -1)
        {
            perror("wiatpid failed");
            return 1;
        }
    }

}