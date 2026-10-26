#ifndef SOCKET_HPP
#define SOCKER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <vector>

class Socket
{
    private:
        int _fd;
    
    public:
        Socket();
        ~Socket();

        bool initializeSocket(int port);
        int getFd() const;
};


#endif