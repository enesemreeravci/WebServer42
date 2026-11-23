#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"

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
#include <cstddef>
#include <map>
#include <string>
#include <cerrno>

class CgiHandler;
class Client;

class Server
{
    private:

        std::vector<int> _listenFds;
        std::vector<struct pollfd> _fds;
        std::map<int, Client> _clients;

        // CGI pipe descriptors and their directions.
        std::map<int, int> _cgiToClientFd;
        // CGI children that still need to be reaped.
        std::vector<CgiHandler*> _pendingReap;

        void acceptClient(int listenFd);
        bool readClient(size_t index);
        void removeClient(size_t index);
        bool writeClient(size_t index);
        bool queueResponse(size_t index, const std::string& response);

        void checkPendingCgi();

        void removePollFd(int fd);
        void enableClientPollout(int clientFd);

        bool isListeninSocket(int fd) const;
        bool startCgiForClient(int clientFd, const std::string& scriptPath,
                                             const std::string& interpreterPath,
                                             const std::vector<std::string>& envVars,
                                             const std::string& requestBody);

        void registerCgiPipe(int pipeFd, int clientFd, short events);

        void cleanupClientCgi(Client& client, bool terminate);
        
        bool HandleCgiPipeEvent(size_t index, size_t& monitoredCount);
        bool HandleClientEvent(size_t index, size_t& monitoredCount);
    
    public:
        Server();
        ~Server();

        bool setupListeninSocket(int port);
        void run();
};

#endif