
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
    
    // enable SO_REUSEADDR before bind()
    // allows the server to reuse an address and port after restarting 
    // even if previous connections are still in TIME_WAIT
    // helps avoid address already in use errors
    // doesnt override another server actively listening on the same port
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(server_fd);
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

    // with vector we will be able to manage multiple clients to connect to our server
    // we can add new clients dynamically and remove them when they disconnect
    std::vector<struct pollfd> fds;
    
    struct pollfd server_pfd;
    server_pfd.fd = server_fd;
    server_pfd.events = POLLIN;
    server_pfd.revents = 0;

    fds.push_back(server_pfd);
    
    //  initilaze the vector before entering the loop
    
    while(true)
    {
        // poll() waits until the listening socket has an event.
        int ready = poll(&fds[0], fds.size(), -1);
                    // pointer to the first pollfd element, number of descriptors to monitor, wait indefinitetl for an event
        
        if (ready == -1)
        {
            perror("poll");
            close(server_fd);
            return 1;
        }
        
        for(unsigned int i = 0; i < fds.size(); i++)
        {
            if (ready > 0 && (fds[i].revents & POLLIN))
            {
                // 
                if(fds[i].fd == server_fd)
                {
                    // accept() returns a new FD for the connected client.
                    int client_fd = accept(server_fd, NULL, NULL);
                    
                    
                    if (client_fd == -1)
                    {
                        perror("accept");
                        continue; // instead of closing the server we want to still continue to receive connection from other clients
                    }
                    // for new client , creating a pollfd structure

                    pollfd client_pfd;
                    client_pfd.fd = client_fd;
                    client_pfd.events = POLLIN;
                    client_pfd.revents = 0;

                    fds.push_back(client_pfd);
                    std::cout << "New client successfully connected, FD: " << client_fd << std::endl;
                }

                else if(fds[i].fd != server_fd)
                {
                    
                    char buffer[100];
                    ssize_t bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    
                    if (bytes <= 0)
                    {
                        if(bytes == 0)
                            std::cout << "CLient has disconnected, FD: " << fds[i].fd << std::endl;
                        else
                            perror("recv error");
                        
                        close(fds[i].fd); // close the socket
                        fds.erase(fds.begin() + i); // remove from the vector
                        i--; // decrease by 1 cause we deleted one element
                    }
                    else
                    {
                        std::cout << "Client FD's: "<< fds[i].fd << std::endl;
                        std::cout << "Message from client: "; 
                        std::cout.write(buffer, bytes);
                        std::cout << "\n";
                        
                        const char *message = "Hello from Webserver";
                        
                        // send() attempts to send bytes through the connected socket.
                        ssize_t bytes_message = send(
                            fds[i].fd, message, strlen(message), 0
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
                            close(fds[i].fd);
                            fds.erase(fds.begin() + i);
                            i--;
                        }
                    }
                    // Close the accepted client socket after communication.
                    //close(client_fd);
                    // uncommented because conenction must be open.
                    // later delete here completely!
            }
        }
    }
            // recv() receives up to sizeof(buffer) bytes.
            // Its return value is the number of bytes actually received.

    }        
    // Close the listening socket before exiting.
    close(server_fd);
    return 0;
}
