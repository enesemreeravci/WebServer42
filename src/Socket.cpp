#include "../include/Socket.hpp"

// we set the _fd -1 to prevent conflict.
Socket::Socket() : _fd(-1) {}

Socket::~Socket()
{
    if(_fd != -1)
        close(_fd);
}

int Socket::getFd() const { return  _fd;}

bool Socket::initializeSocket(int port)
{

    if(port < 1 || port > 65535)
        return false;
    if(_fd != -1)
        return false;

    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if(_fd == -1)
    {
        perror("Socket failed");
        return false;
    }

    int opt = 1;
    if(setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(_fd);
        _fd = -1; // we set to -1 to indicate our socker object no longer owns a valid file descriptor
        return false;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    struct sockaddr* generic_addr = reinterpret_cast<struct sockaddr * >(&addr);
    std::cout << "Address succesfully initialized" << std::endl;
    
    if(bind(_fd, generic_addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        close(_fd);
        _fd = -1;
        return false;
    }

    std::cout << "Socket bound to 127.0.0.1:" << port <<  std::endl;

    if(listen(_fd, 5) < 0)
    {
        perror("listen() failed");
        close(_fd);
        _fd = -1;
        return false;
    }

    std::cout << "Server is listening on port" << port <<  std::endl;
    
    // gets the current file status flags
    int flags = fcntl(_fd, F_GETFL);
    if(flags == -1)
    {
        perror("fcntl F_GETFL");
        close(_fd);
        _fd = -1;
        return false;
    }

    // F_SETFL updates the flags
    // O_NONBLOCK prevent socker operations from waiting for I/O
    int result = fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
    if(result == -1)
    {
        perror("fcntl F_SETFL");
        close(_fd);
        _fd = -1;
        return false;
    }

    return true;
    
}