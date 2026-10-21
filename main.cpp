
#include "sockets.hpp"

int main()
{
    // socket() creates a socket and returns its file descriptor.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        return 1;
    }

    std::cout
        << "Socket created successfully!\n"
        << "Socket's fd = " << server_fd << std::endl;

    // sockaddr_in stores an IPv4 address and port.
    // htons() converts a 16-bit value to network byte order.
    // htonl() converts a 32-bit value to network byte order.
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // bind() expects a pointer to the generic sockaddr type.
    struct sockaddr* generic_addr = reinterpret_cast<struct sockaddr *>(&addr);
    std::cout << "Address successfully initialized" << std::endl;

    // bind() assigns an IP address and port to the socket.
    if (bind(server_fd, generic_addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }
    std::cout << "Socket bound to 127.0.0.1:8080" << std::endl;

    // listen() makes the socket ready for incoming connections.
    // 5 is the requested backlog size.
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }
    std::cout << "Server is listening on port 8080" << std::endl;

    // fcntl(F_GETFL) gets the current file status flags.
    int flags = fcntl(server_fd, F_GETFL);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        close(server_fd);
        return 1;
    }

    // fcntl(F_SETFL) updates the flags.
    // O_NONBLOCK prevents socket operations from waiting for I/O.
    int result = fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    if (result == -1)
    {
        perror("fcntl F_SETFL");
        close(server_fd);
        return 1;
    }

    // pollfd describes which FD and events we want to monitor.
    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    // poll() waits until the listening socket has an event.
    int ready = poll(&pfd, 1, -1);

    if (ready == -1)
    {
        perror("poll");
        close(server_fd);
        return 1;
    }

    if (ready > 0 && (pfd.revents & POLLIN))
    {
        // accept() returns a new FD for the connected client.
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd == -1)
        {
            perror("accept");
            close(server_fd);
            return 1;
        }

        std::cout
            << "Client connected successfully!\n"
            << "Client's fd = " << client_fd << std::endl;

        // recv() receives up to sizeof(buffer) bytes.
        // Its return value is the number of bytes actually received.
        char buffer[100];
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes < 0)
        {
            perror("Recv failed");
            close(client_fd);
            close(server_fd);
            return 1;
        }
        else if (bytes == 0)
        {
            std::cout << "Client closed its sending side" << std::endl;
        }
        else
        {
            std::cout << "Message from client: ";
            std::cout.write(buffer, bytes);
            std::cout << "\n";

            const char *message = "Hello from Webserver";

            // send() attempts to send bytes through the connected socket.
            ssize_t bytes_message = send(
                client_fd, message, strlen(message), 0
            );

            if (bytes_message > 0)
            {
                std::cout << "Server response: ";
                std::cout.write(message, bytes_message);
                std::cout << "\n";
            }
            else if (bytes_message == 0)
            {
                std::cout << "bytes_message = 0" << std::endl;
            }
            else
            {
                perror("Send failed");
                close(client_fd);
                close(server_fd);
                return 1;
            }
        }

        // Close the accepted client socket after communication.
        close(client_fd);
    }

    // Close the listening socket before exiting.
    close(server_fd);
    return 0;
}
