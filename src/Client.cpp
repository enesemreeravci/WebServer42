#include "../include/Client.hpp"

Client::Client(int fd) : _fd(fd), _bytesSent(0), _responseQueued(false) {}

Client::~Client()
{

}

int Client::getClientFd() const { return _fd; }

const std::string& Client::getOutputBuffer() const { return _outputBuffer; }

size_t Client::getBytesSent() const { return _bytesSent; }

bool Client::isResponseQueued() const { return _responseQueued; }

void Client::markResponseQueued() 
{
    _responseQueued = true;
}

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

const std::string& Client::getInputBuffer() const { return _inputBuffer; }

// we append data because TCP may deliver one HTTP request thorugh multiple recv() class
void Client::appendInput(const char *data, size_t length)
{
    _inputBuffer.append(data, length);
}

// stores a new response and resets the number of bytes already sent
void Client::setOutput(const std::string &response)
{
    _outputBuffer = response;
    _bytesSent = 0;
}

// updates sending progress after bytes have been succesfully transmitted
void Client::advanceSent(size_t bytes)
{
    _bytesSent += bytes;
}