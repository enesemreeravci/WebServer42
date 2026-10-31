#ifndef SERVER_HPP
#define SERVER_HPP

#include "Socket.hpp"
#include "Client.hpp"
#include <map>
#include <vector>
#include <poll.h>
#include <cerrno>

class Server
{
    private:
        Socket _socket; // (owns)responsible for creating listening socket and clean up
        std::vector<struct pollfd> _fds; // shortly, tells poll() which sockets to monitor 
                                        // keeps track of the file descriptors that poll() monitors. 
                                        // It contains the listening socket FD as well as connected client FDs.
        std::map<int, Client> _clients; // the client's file descriptor = the object containin that client's buffers and state
        void acceptClient();
        bool readClient(size_t index); // index for finding which client we want to read from, we are saying "read from the client stored at index 3"
        void removeClient(size_t index);
        bool writeClient(size_t index);
        bool queueResponse(size_t index, const std::string& response);
        
    public:
        Server();
        ~Server();

        bool initializeServer(int port);
        void run();
}; 

#endif