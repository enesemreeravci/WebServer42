#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/wait.h>


/*
Parent process
    |
    | write("Hello from parent")
    v
  PIPE
    |
    v
Child process
    |
    | read()
    v
Print received message
*/

int main()
{
    int pipefd[2];

    if(pipe(pipefd) == -1)
    {
        std::cerr << "error pipe" << std::endl;
        return 1;
    }
    
    pid_t c_pid = fork();
    if(c_pid == -1)
    {
        std::cerr << "error fork" << std::endl;
        close(pipefd[0]); // reading
        close(pipefd[1]); // writing
        return 1;
    }
    if(c_pid > 0)
    {
        close(pipefd[0]);
        const char *buf = "hello form parent";
        ssize_t bytesWritten = write(pipefd[1], buf, strlen(buf));
        if(bytesWritten == -1)
        {
            std::cout << "write error" << std::endl;
        }
        close(pipefd[1]);

        int status;
        pid_t res = waitpid(c_pid, &status, 0);
        if(res == -1)
        {
            std::cout << "error waitpid" << std::endl;
        }
    }
    else if(c_pid == 0)
    {
        close(pipefd[1]);
        if(dup2(pipefd[0], STDIN_FILENO) == -1)
        {
            close(pipefd[0]);          
            _exit(1);
        }
        char buffer[100];
        ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) -1 );
        if(bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            std::cout << buffer << std::endl;
        }
        _exit(0);
    }

}