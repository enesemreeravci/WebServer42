#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <cstddef>

class Client
{
    private:
        int _fd; // connected socket descriptor
        std::string _inputBuffer; // stores recevied request bytes
        std::string _outputBuffer; // stores  response bytes waiting to be sent
        size_t _bytesSent; // tracks how much of the response has been transmitted


    public:
        Client(int fd);
        ~Client();
        void appendInput(const char *data, size_t length);
        void setOutPut(const std::string& response);
        void advanceSent(size_t bytes);

        int getClientFd() const;
        const std::string& getOutputBuffer() const;
        size_t getBytesSent() const;
        bool hasPendingOutput() const;
};

#endif