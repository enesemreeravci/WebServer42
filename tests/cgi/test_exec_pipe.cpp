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

    int outputPipe[2];
    if(pipe(outputPipe) == -1)
    {
        std::cerr << "Pipe failed" << std::endl;
        return 1;
    }
    pid_t c_pid = fork();
    if(c_pid == -1)
    {
        close(outputPipe[0]);
        close(outputPipe[1]);
        return 1;
    }
    if(c_pid == 0)
    {
        close(outputPipe[0]); // child only writes
        if(dup2(outputPipe[1], STDOUT_FILENO) == -1)
        {
            close(outputPipe[1]);
            _exit(1);
        }
        close(outputPipe[1]);

        if(execve("/bin/echo", argv, envp) == -1)
        {
            perror("execve failed");
            _exit(1);
        }
    }
    else if(c_pid > 0)
    {
        close(outputPipe[1]); // parent only reads;
        char buffer[100];
        
        ssize_t bytesRead = read(outputPipe[0], buffer, sizeof(buffer) - 1);
        if(bytesRead == -1)
        {
            perror("read failed");
        }
        if(bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            std::cout << buffer << std::endl;
        }

        close(outputPipe[0]); 
        int status;
        pid_t  result = waitpid(c_pid, &status, 0);
        if(result == -1)
        {
            perror("waitpid failed");
        }
    }


}