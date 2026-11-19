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

        void registerCgiFd(int pipeFd, int clientFd, bool isInput);
        void unregisterCgiFd(int fd);
        void checkPendingCgi();
        void removeCgi(int clientFd);
        void enableClientPollout(int clientFd);

        bool isListeninSocket(int fd) const;

        void removePollFd(int fd);
        bool startCgiForClient(int clientFd, const std::string& scriptPath,
                                             const std::string& interpreterPath,
                                             const std::vector<std::string>& envVars,
                                             const std::string& requestBody);


    public:
        Server();
        ~Server();

        bool setupListeninSocket(int port);
        void run();
};

#endif