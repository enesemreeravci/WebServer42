#include "../../include/network/Server.hpp"
#include "../../include/cgi/CgiHandler.hpp"
#include "../../include/utils/Utils.hpp"

Server::Server() {}
Server::~Server() 
{
    for(size_t i = 0; i < _fds.size(); i++)
    {
        close(_fds[i].fd);
    }
}


bool Server::setupListeninSocket(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return false;

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        close(fd);
        return false;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //or INADDR_LOOPBACK

    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0 ||
        listen(fd, SOMAXCONN) < 0 ||
        (!Utils::setNonBlocking(fd)))
    {
        close(fd);
        return false;
    }

    _listenFds.push_back(fd);

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _fds.push_back(pfd);
    return true;
}


bool Server::isListeninSocket(int fd) const
{
    for(size_t i = 0; i < _listenFds.size(); i++)
    {
        if(_listenFds[i] == fd)
            return true;
    }
    return false;
}

// handle incoming connections
void Server::acceptClient(int listenFd)
{
    int client_fd = accept(listenFd, NULL, NULL);
    if (client_fd == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            perror("Accept");
        return;
    }

    if (!Utils::setNonBlocking(client_fd))
    {
        close(client_fd);
        return;
    }

    Client client_obj(client_fd);
    if (!_clients.insert(std::make_pair(client_fd, client_obj)).second)
    {
        close(client_fd);
        return;
    }

    pollfd client_pfd;
    client_pfd.fd = client_fd;
    client_pfd.events = POLLIN;
    client_pfd.revents = 0;
    _fds.push_back(client_pfd);

    std::cout << "New client has successfully connected, FD is: " << client_fd << std::endl;
}

//handle existing clients   
bool Server::readClient(size_t index)
{
    char buffer[1024];
    
    int client_fd = _fds[index].fd; // gets the client's fd from _fds
    std::map<int, Client>::iterator it; 

    it = _clients.find(client_fd); // it points to the matching map entry if exists
    if(it == _clients.end()) // if it is not found return false
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
        if (position != std::string::npos && !it->second.isResponseQueued())
        {
            std::cout << "HTTP header terminator found" << std::endl;

            // GEÇİCİ CGI TEST BLOĞU
            if (input.find("GET /cgi-bin/test.py") != std::string::npos && it->second.getCgi() == NULL)
            {
                std::vector<std::string> envVars;
                envVars.push_back("REQUEST_METHOD=GET");
                envVars.push_back("QUERY_STRING=user=enes");
                envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
                envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");

                // Not: startProcess içinde chdir("./www/cgi-bin") yapıldığı için
                // python3'e script adı olarak "test.py" veya tam yol verilmeli.
               std::cout << "[DEBUG] CGI bloguna girildi!" << std::endl;
                    if (startCgiForClient(client_fd, "www/cgi-bin/test.py", "/usr/bin/python3", envVars, ""))
                    {
                        return true;
                    }
                    std::cout << "[DEBUG] startCgiForClient FALSE dondu!" << std::endl;
            
            }

            const std::string fixedHttpResponse =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 5\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "Hello";
            if (!queueResponse(index, fixedHttpResponse))
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

    removeCgi(client_fd); // it will remove cgi client
    _clients.erase(client_fd);      // removes the client's stored data and state
    close(_fds[index].fd); // relases the os's socker descriptor
    
    for(size_t i = 0; i < _fds.size(); ++i)
    {
        if(_fds[i].fd == client_fd)
        {
            _fds.erase(_fds.begin() + i); // stops our event loop from monitoring that FD
            break;
        }    

    }
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
            client.reset();
            return true; // keep the connection open 
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

// add a cgi fd to both type and _fds
// so the event loop knows its type and can monitor it
void Server::registerCgiFd(int fd, int clientFd, bool isInput)
{
    _cgiToClientFd[fd] = clientFd;

    struct pollfd cgi;
    cgi.fd = fd;
    if(isInput)
        cgi.events = POLLOUT;
    else 
        cgi.events = POLLIN;
    cgi.revents = 0;

    _fds.push_back(cgi);
}

void Server::unregisterCgiFd(int fd)
{
    _cgiToClientFd.erase(fd);

    std::vector<struct pollfd>::iterator it = _fds.begin();
    while(it != _fds.end())
    {
        if(it->fd == fd)
        {
            _fds.erase(it);
            break;
        }
        it++;
    }
}

void Server::enableClientPollout(int clientFd)
{
    for(size_t i = 0; i < _fds.size(); ++i)
    {
        if(_fds[i].fd == clientFd)
        {
            _fds[i].events |= POLLOUT;
            break;
        }
    }
}

void Server::checkPendingCgi()
{
    std::map<int, Client>::iterator it;
    
    for(it = _clients.begin(); it != _clients.end(); ++it)
    {
        int clientFd = it->first;
        Client& client = it->second;
        CgiHandler* cgi = client.getCgi();
        if(cgi == NULL)
            continue;
        
        if(cgi->isTimedOut(10)) // timeout check, 10 sec
        {
            if(cgi->getInputFd() != -1)
                removePollFd(cgi->getInputFd());
            if(cgi->getOutputFd() != -1)
                removePollFd(cgi->getOutputFd());
            
            cgi->terminateChild();
            delete cgi;
            client.setCgi(NULL);

            std::string timeoutReap = 
                "HTTP/1.1 504 Gateway Timeout\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 19\r\n\r\n"
                "504 Gateway Timeout";

            client.setOutput(timeoutReap);
            client.markResponseQueued();
            enableClientPollout(clientFd);
            continue;
        }

        // if output pipe has closed, EOF
        if(cgi->getOutputFd() == -1)
        {
            ReapResult status = cgi->reapChild();
            if(status == REAP_FINISHED || status == REAP_ERROR)
            {
                std::string cgiOutput = cgi->getOutputBuffer();
                delete cgi;
                client.setCgi(NULL);

                std::string response;
                if(status == REAP_ERROR)
                {
                    response =
                    "HTTP/1.1 502 Bad Gateway\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 15\r\n\r\n"
                    "502 Bad Gateway";
                }
                else
                {
                    response = "HTTP/1.1 200 OK\r\nConnection: close\r\n" + cgiOutput;
                }
            
                client.setOutput(response);
                client.markResponseQueued();
                enableClientPollout(clientFd);
            }
        }
    }
}
void Server::removePollFd(int fd)
{
    _cgiToClientFd.erase(fd);
    for(size_t i = 0; i < _fds.size(); i++)
    {
        if(_fds[i].fd == fd)
        {
            _fds.erase(_fds.begin() + i);
            break;
        } 
    }
}

bool Server::startCgiForClient(int clientFd, const std::string& scriptPath,
                                             const std::string& interpreterPath,
                                             const std::vector<std::string>& envVars,
                                             const std::string& requestBody)
{
    if(_clients.find(clientFd) == _clients.end())
        return false;

    std::map<int, Client>::iterator it = _clients.find(clientFd);
    if(it == _clients.end())
        return false;
    Client& client = it->second;

    if(client.getCgi() != NULL)
        return false;    // already there is active cgi client

    CgiHandler* cgi = new CgiHandler();
    cgi->setInputBuffer(requestBody);

    if(!cgi->startProcess(scriptPath, interpreterPath, envVars))
    {
        delete cgi;
        return false;
    }

    client.setCgi(cgi);

    if(cgi->getInputFd() != -1)
    {
        struct pollfd pfd;
        pfd.fd = cgi->getInputFd();
        pfd.events = POLLOUT;
        pfd.revents = 0;
        _fds.push_back(pfd);
        _cgiToClientFd[(cgi->getInputFd())]  = clientFd;
    }

    if(cgi->getOutputFd() != -1)
    {
        struct pollfd pfd;
        pfd.fd = cgi->getOutputFd();
        pfd.events = POLLIN;
        pfd.revents = 0;
        _fds.push_back(pfd);
        _cgiToClientFd[(cgi->getOutputFd())] = clientFd;
    }
    return true;
}


void Server::removeCgi(int clientFd)
{
    std::map<int, Client>::iterator clientIt = _clients.find(clientFd);
    if (clientIt == _clients.end())
        return;

    Client& client = clientIt->second;
    CgiHandler* cgi = client.getCgi();
    if (cgi == NULL)
        return;

    int inputfd = cgi->getInputFd();
    int outputfd = cgi->getOutputFd();

    if (inputfd != -1)
        unregisterCgiFd(inputfd);
    if (outputfd != -1)
        unregisterCgiFd(outputfd);

    ReapResult res = cgi->reapChild();
    if (res == REAP_RUNNING)
    {
        std::cout << "CGI child still running" << std::endl;
        if (!cgi->isCancelRequested())
        {
            if (!cgi->terminateChild())
                perror("SIGTERM failed");
        }
        _pendingReap.push_back(cgi);
    }
    else if (res == REAP_FINISHED)
    {
        std::cout << "CGI child successfully reaped" << std::endl;
        delete cgi;
    }
    else
    {
        std::cout << "CGI child reap error" << std::endl;
        delete cgi;
    }

    client.setCgi(NULL);
}

void Server::run()
{
    while(true)
    {
        int readyFds = poll(&_fds[0], _fds.size(), 100);

        if(readyFds == -1)
        {
            if(errno == EINTR)
                continue;
            perror("POLL");
            return;
        }

        checkPendingCgi();

        size_t monitoredCount = _fds.size();

        for(size_t i =0; i < monitoredCount; )
        {
            int currentFd = _fds[i].fd;
            short revents = _fds[i].revents;
            bool shouldRemoveCLient = false;

            // cgi pipe things pollin pollout pollhup pollerr
            if(_cgiToClientFd.find(currentFd) != _cgiToClientFd.end())
            {
                int clientFd = _cgiToClientFd[currentFd];
                std::map<int, Client>::iterator clientIt = _clients.find(clientFd);
                CgiHandler* cgi = (clientIt != _clients.end()) ? clientIt->second.getCgi() : NULL;
            
                if(cgi == NULL)
                {
                    removePollFd(currentFd);
                    --monitoredCount;
                    continue;
                }

                if((revents & POLLOUT) && currentFd == cgi->getInputFd())
                {
                    if(cgi->writeToStdin())
                    {
                        removePollFd(currentFd);
                        --monitoredCount;
                        continue;
                    }
                }

                if((revents & (POLLIN | POLLHUP | POLLERR)) && currentFd == cgi->getOutputFd())
                {
                    if(cgi->readFromStdout())
                    {
                        removePollFd(currentFd);
                        --monitoredCount;
                        continue;
                    }
                }
                ++i;
                continue;
            }
            // new client connections
            if((revents & POLLIN) && isListeninSocket(currentFd))
            {
                acceptClient(currentFd);
                ++i;
                continue;
            }

            //client sockets
            if(_clients.find(currentFd) != _clients.end())
            {
                if(revents & POLLIN)
                {
                    if(!readClient(i))
                        shouldRemoveCLient = true;
                }

                if((revents & POLLOUT) && !shouldRemoveCLient)
                {
                    if(!writeClient(i))
                        shouldRemoveCLient = true;
                }

                if((revents & (POLLERR | POLLHUP | POLLNVAL)) && !(revents & POLLIN))
                {
                    shouldRemoveCLient = true;
                }

                if(shouldRemoveCLient)
                {
                    removeClient(i);
                    --monitoredCount;
                    continue;
                }
            }
            ++i;
        }
    }
}