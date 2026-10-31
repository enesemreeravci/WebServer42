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
            
            // std::make_pair creates a pair (first = client_id, second_client_obj)
            // insert() returns another pair
            // we are storing successufuy created pair in our map which is _clients
            bool inserted = _clients.insert(std::make_pair(client_fd, client_obj)).second;
            // if key is already existed , no new entry was insreted and close the fd
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
    
    int client_fd = _fds[index].fd; // gets the client's fd from _fds
    std::map<int, Client>::iterator it; 

    it = _clients.find(client_fd); // it points to the matching map entry if exists
    if(it == _clients.end()) // it it is not found return false
        return false;

    ssize_t bytesReceived = recv(_fds[index].fd, buffer, sizeof(buffer), 0);
    
    if (bytesReceived > 0)
    {
        std::cout << "Client's FD is: " << _fds[index].fd << std::endl;
        std::cout << "Message from client: ";
        std::cout.write(buffer, bytesReceived);
        std::cout << std::endl;
        // it->first gives us client's FD(int)
        // it->second gives us exisiting Client object
        it->second.appendInput(buffer, bytesReceived); // accumulates recevied bytes from multiple recv() calls
        const std::string& input =  it->second.getInputBuffer(); // get the accumalted inputbuffer
        size_t position = input.find("\r\n\r\n"); // search for the end of HTTP headers
        if(position != std::string::npos && !it->second.isResponseQueued()) // did we find the end of the http headears?
        {
            std::cout << "HTTP header terminator found" << std::endl;
            const std::string fixedHttpResponse =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 5\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "Hello";
            if(!queueResponse(index, fixedHttpResponse))
            {
                std::cerr << "Could not queue response for FD " << client_fd << std::endl;
            }
            else
            {
                it->second.markResponseQueued();
            }
        
        }
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
    std::cout << "Server closing client FD: " << client_fd << std::endl;
    _clients.erase(client_fd);      // removes the client's stored data and state
    close(_fds[index].fd); // relases the os's socker descriptor
    _fds.erase(_fds.begin() + index); // stops our event loop from monitoring that FD
}

bool Server::queueResponse(size_t index, const std::string& response)
{
    int client_fd  = _fds[index].fd;

    std::map<int, Client>::iterator it;
    it = _clients.find(client_fd);
    if(it == _clients.end()) return false;

    Client& client = it->second;

    if(client.hasPendingOutput() == true)
        return false;

    if(response.empty())
        return false;
    client.setOutput(response);
    _fds[index].events |= POLLOUT; // enabling POLLOUT
    return true;
}

bool Server::writeClient(size_t index)
{
    int client_fd = _fds[index].fd;

    std::map<int, Client>::iterator it;
    
    it = _clients.find(client_fd);
    if(it == _clients.end())
        return false;

    Client& client = it->second;
    if(client.hasPendingOutput() == false)
        return true;
    const std::string& response = client.getOutputBuffer();
    size_t bytesSent = client.getBytesSent();

    size_t remainingBytes;
    remainingBytes = response.size() - bytesSent;

    ssize_t bytesWritten;                                                   //prevent sigpipe 
    bytesWritten = send(client_fd, response.c_str() + bytesSent, remainingBytes, MSG_NOSIGNAL);
    
    if(bytesWritten > 0)
    {
        client.advanceSent(bytesWritten);
        if(!client.hasPendingOutput()) // if it returns false. all response bytes have been sent
        {
            _fds[index].events &= ~POLLOUT; // if no bytes remain, events &= ~pollout stops monitoring this cleitn for write readiness
            return false;
        }  
        return true;
    }
    else if (bytesWritten == 0)
    {
       // std::cout << "do nothing";
        return true;
    }
    else 
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            return true; 
        perror("send");
        return false;
    }
    

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
            
            // pollin meeans socket is ready for a read operation
            if(_fds[i].revents & POLLIN)
            {
                std::cout << "FD " << _fds[i].fd
                          << " is ready for reading" << std::endl;

                if(_fds[i].fd == _socket.getFd())
                    acceptClient();
                else if(!readClient(i))
                    shouldRemoveClient = true;
            }
            if(_fds[i].revents & POLLOUT)
            {
                std::cout << "FD " <<  _fds[i].fd << " is ready to write" << std::endl;
                
                if(_fds[i].fd != _socket.getFd() && !shouldRemoveClient)
                {
                    if(writeClient(i) == false)
                        shouldRemoveClient = true;
                }
            }
            // POLLIN - A read operation can proceed
            // POLLOUT - writing is now possible
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