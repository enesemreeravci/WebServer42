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

        void acceptClient();
        bool readClient(size_t index); // index for finding which client we want to read from, we are saying "read from the client stored at index 3"
        void removeClient(size_t index);

    public:
        Server();
        ~Server();

        bool initializeServer(int port);
        void run();
}; 

#endif