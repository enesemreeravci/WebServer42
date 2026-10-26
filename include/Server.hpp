#ifndef SERVER_HPP
#define SERVER_HPP

#include "Socket.hpp"
#include <vector>
#include <poll.h>
#include <cerrno>

class Server
{
    private:
        Socket _socket; // (owns)responsible for creating listening socket and clean up
        std::vector<struct pollfd> _fds; // keeps track of the file descriptors that poll() monitors. 
                                        // It contains the listening socket FD as well as connected client FDs.
    public:
        Server();
        ~Server();

        bool initializeServer(int port);
        void run();
}; 

#endif