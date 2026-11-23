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
    std::cout << "Server closing client Fd: " << client_fd << std::endl;

    std::map<int, Client>::iterator it = _clients.find(client_fd);

    if(it != _clients.end())
    {
        cleanupClientCgi(it->second, true);
        _clients.erase(it);
    }
    removePollFd(client_fd);
    close(client_fd);
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

void Server::registerCgiPipe(int pipeFd, int clientFd, short events)
{
    if(pipeFd == -1)
        return;

    struct pollfd pfd;
    pfd.fd = pipeFd;
    pfd.events = events;
    pfd.revents = 0;

    _fds.push_back(pfd);

    _cgiToClientFd[pipeFd] = clientFd;
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
            cleanupClientCgi(client, true);

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
                std::string cgiOutput = cgi->getOutputBuffer(); // copy the ouput
                cleanupClientCgi(client, false); // clean and termiante it

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

void Server::cleanupClientCgi(Client& client, bool terminate)
{
    CgiHandler* cgi = client.getCgi();

    if(cgi == NULL)
        return;

    if(cgi->getInputFd() != -1)
        removePollFd(cgi->getInputFd());
    if(cgi->getOutputFd() != -1)
        removePollFd(cgi->getOutputFd());

    if(terminate)
        cgi->terminateChild();
    delete cgi;
    client.setCgi(NULL);
    
}

bool Server::HandleCgiPipeEvent(size_t index, size_t& monitoredCount)
{
    int currentFd = _fds[index].fd;
    short revents = _fds[index].revents;

    int clientFd = _cgiToClientFd[currentFd];
    std::map<int, Client>::iterator clientIt = _clients.find(clientFd);
    CgiHandler* cgi = (clientIt != _clients.end()) ? clientIt->second.getCgi() : NULL;

    if (cgi == NULL)
    {
        removePollFd(currentFd);
        --monitoredCount;
        return true;
    }

    if ((revents & POLLOUT) && currentFd == cgi->getInputFd())
    {
        if (cgi->writeToStdin())
        {
            removePollFd(currentFd);
            --monitoredCount;
            return true;
        }
    }

    if ((revents & (POLLIN | POLLHUP | POLLERR)) && currentFd == cgi->getOutputFd())
    {
        if (cgi->readFromStdout())
        {
            removePollFd(currentFd);
            --monitoredCount;
            return true;
        }
    }

    return false;
}

bool Server::HandleClientEvent(size_t index, size_t& monitoredCount)
{
    short revents = _fds[index].revents;
    bool shouldRemoveClient = false;

    if (revents & POLLIN)
    {
        if (!readClient(index))
            shouldRemoveClient = true;
    }

    if ((revents & POLLOUT) && !shouldRemoveClient)
    {
        if (!writeClient(index))
            shouldRemoveClient = true;
    }

    if ((revents & (POLLERR | POLLHUP | POLLNVAL)) && !(revents & POLLIN))
    {
        shouldRemoveClient = true;
    }

    if (shouldRemoveClient)
    {
        removeClient(index);
        --monitoredCount;
        return true;
    }

    return false;
}

bool Server::startCgiForClient(int clientFd, const std::string& scriptPath,
                                             const std::string& interpreterPath,
                                             const std::vector<std::string>& envVars,
                                             const std::string& requestBody)
{

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

    registerCgiPipe(cgi->getInputFd(), clientFd, POLLOUT);
    registerCgiPipe(cgi->getOutputFd(), clientFd, POLLIN);
    return true;
}


void Server::run()
{
    while (true)
    {
        int readyFds = poll(&_fds[0], _fds.size(), 100);

        if (readyFds == -1)
        {
            if (errno == EINTR)
                continue;
            perror("POLL");
            return;
        }

        checkPendingCgi();

        size_t monitoredCount = _fds.size();

        for (size_t i = 0; i < monitoredCount; )
        {
            int currentFd = _fds[i].fd;

            if (_cgiToClientFd.find(currentFd) != _cgiToClientFd.end())
            {
                if (HandleCgiPipeEvent(i, monitoredCount))
                    continue;
            }
            else if ((_fds[i].revents & POLLIN) && isListeninSocket(currentFd))
            {
                acceptClient(currentFd);
            }
            else if (_clients.find(currentFd) != _clients.end())
            {
                if (HandleClientEvent(i, monitoredCount))
                    continue;
            }
            ++i;
        }
    }
}