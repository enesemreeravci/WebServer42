#include "../include/Client.hpp"

Client::Client(int fd) : _fd(fd), _bytesSent(0) {}

Client::~Client()
{

}

int Client::getClientFd() const { return _fd; }

const std::string& Client::getOutputBuffer() const { return _outputBuffer; }

size_t Client::getBytesSent() const { return _bytesSent; }

bool Client::hasPendingOutput() const 
{
    // if the buffer contains more btes than we have already sent
    // means there is still data waiting
    if(_outputBuffer.size() > getBytesSent())
    {
        return true;
    }
    return false;
}