#include "../include/Server.hpp"

Server::Server() {}
Server::~Server() {}

bool Server::initializeServer(int port)
{
    if(!_socket.initializeSocket(port))
        return false;

    struct pollfd server_pfd;
    server_pfd.fd = _socket.getFd();
    server_pfd.events = POLLIN;
    server_pfd.revents = 0;

    _fds.push_back(server_pfd);

    return true;
}


void Server::run()
{
    while(true)
    {
        int readyFds = poll(&_fds[0], _fds.size(), -1);

        if(readyFds == -1)
        {
            if(errno == EINTR)
                continue;
            perror("poll");
            return;
        }

        // Number of FDs that existed when poll() returned
        size_t monitoredCount = _fds.size();

        for(size_t i = 0; i < monitoredCount; )
        {
            bool removeClient = false;

            if(_fds[i].revents & POLLIN)
            {
                std::cout << "FD " << _fds[i].fd
                          << " is ready for reading" << std::endl;

                // new clients
                if(_fds[i].fd == _socket.getFd())
                {
                    // accept() returns a new FD for the connected client
                    int client_fd = accept(_socket.getFd(), NULL, NULL);

                    if(client_fd == -1)
                    {
                        if(errno != EAGAIN && errno != EWOULDBLOCK)
                            perror("Accept");
                    }
                    else
                    {
                        // Retrieve the existing file status flags
                        int flags = fcntl(client_fd, F_GETFL);

                        if(flags == -1)
                        {
                            perror("fcntl() F_GETFL");
                            close(client_fd);
                        }
                        else
                        {
                            // Enable non-blocking mode
                            int result = fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                            if(result == -1)
                            {
                                perror("fcntl F_SETFL");
                                close(client_fd);
                            }
                            else
                            {
                                pollfd client_pfd;
                                client_pfd.fd = client_fd;
                                client_pfd.events = POLLIN;
                                client_pfd.revents = 0;

                                _fds.push_back(client_pfd);

                                std::cout << "New client succesfully connected with FD: "
                                          << client_fd << std::endl;
                            }
                        }
                    }
                }
                // Existing clients
                else
                {
                    char buffer[1024];

                    ssize_t bytesReceived = recv(
                        _fds[i].fd, buffer, sizeof(buffer), 0
                    );

                    if(bytesReceived > 0)
                    {
                        std::cout << "Client FD's: " << _fds[i].fd << std::endl;
                        std::cout << "Message from client: ";
                        std::cout.write(buffer, bytesReceived);
                        std::cout << "\n";
                    }
                    else if(bytesReceived == 0)
                    {
                        std::cout << "Client disconnected, FD: "
                                  << _fds[i].fd << std::endl;

                        removeClient = true;
                    }
                    else
                    {
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            // No data available, keep client connected
                        }
                        else
                        {
                            perror("Recv");
                            removeClient = true;
                        }
                    }
                }
            }
            if(removeClient)
            {
                close(_fds[i].fd);
                _fds.erase(_fds.begin() + i);
                --monitoredCount;
                continue;
            }

            ++i;
        }
    }
}