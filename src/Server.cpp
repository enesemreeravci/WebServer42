#include "../include/Server.hpp"

Server::Server() {}
Server::~Server() 
{
    for(size_t i = 0; i < _fds.size(); i++)
    {
        if(_fds[i].fd != _socket.getFd())
            close(_fds[i].fd);
    }
}

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

// handle incoming connections
void Server::acceptClient()
{
    // for new client it will return a new FD
    int client_fd = accept(_socket.getFd(), NULL, NULL);

    
    if(client_fd == -1)
    {
        if(errno != EAGAIN && errno != EWOULDBLOCK)
        perror("Accept");
        return;
    }
    // if accept() successfull run here
    Client client_obj(client_fd);

    int flags = fcntl(client_fd, F_GETFL);
    if(flags == -1)
    {
        perror("fcntl() F_GETFL");
        close(client_fd);
        return;
    }
    else
    {
        int result = fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        if(result == -1)
        {
            perror("fcntl() F_SETFL");
            close(client_fd);
            return;
        }
        else
        {
            pollfd client_pfd;
            client_pfd.fd = client_fd;
            client_pfd.events = POLLIN;
            client_pfd.revents = 0;
            
            // std::make_pair(client_fd, client_obj);
            bool inserted = _clients.insert(std::make_pair(client_fd, client_obj)).second;

            if(inserted == false)
            {
                close(client_fd);
                return;
            }
            _fds.push_back(client_pfd);

            std::cout << "New client has successfully connected, FD is: " << client_fd << std::endl;
        }
    }

}

//handle existing clients   
bool Server::readClient(size_t index)
{
    char buffer[1024];
    
    ssize_t bytesReceived = recv(_fds[index].fd, buffer, sizeof(buffer), 0);
    
    if (bytesReceived > 0)
    {
        std::cout << "Client's FD is: " << _fds[index].fd << std::endl;
        std::cout << "Message from client: ";
        std::cout.write(buffer, bytesReceived) << std::endl; 
        return true;
    }
    else if(bytesReceived == 0)
    {
        std::cout << "Client disconnected, FD: " << _fds[index].fd << std::endl;
        return false;
    }
    else 
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // no data available, keep the client connected
            return true;
        }
        perror("Recv");
        return false;
    }
    
}

void Server::removeClient(size_t index)
{
    int client_fd = _fds[index].fd; 
    _clients.erase(client_fd);      // removes the client's stored data and state
    close(_fds[index].fd); // relases the os's socker descriptor
    _fds.erase(_fds.begin() + index); // stops our event loop from monitoring that FD
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
            bool shouldRemoveClient = false;

            if(_fds[i].revents & POLLIN)
            {
                std::cout << "FD " << _fds[i].fd
                          << " is ready for reading" << std::endl;

                if(_fds[i].fd == _socket.getFd())
                    acceptClient();
                else if(!readClient(i))
                    shouldRemoveClient = true;
            }
            // POLLIN - A read operation can proceed
            // POLLHUP - A hangup has occurred
            // POLLERR - A socket error occurred
            // POLLNVAL - THe file descripto is invalid
            // we inspect revents because multiple flags can appear together
            // it handles client disconnection and error events
            if(_fds[i].fd != _socket.getFd() && 
            (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) && 
            !(_fds[i].revents & POLLIN))
            {
                shouldRemoveClient = true;
            }
            if(shouldRemoveClient)
            {
                removeClient(i);
                --monitoredCount;
                continue;
            }
            ++i;
        }
    }
}