#include "Socket.hpp"

int main()
{

    Socket server;
    if(!server.initializeSocket(8080))
    {
        std::cerr << "Server initialiaztion failed" << std::endl;
        return 1;
    }

    // with vector we will be able to manage multiple clients to connect to our server
    // we can add new clients dynamically and remove them when they disconnect
    std::vector<struct pollfd> fds;
    
    struct pollfd server_pfd;
    server_pfd.fd = server.getFd();
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
            return 1;
        }
        
        for(unsigned int i = 0; i < fds.size(); i++)
        {
            if (ready > 0 && (fds[i].revents & POLLIN))
            {
                // 
                if(fds[i].fd == server.getFd())
                {
                    // accept() returns a new FD for the connected client.
                    int client_fd = accept(server.getFd(), NULL, NULL);
                    
                    
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

                else if(fds[i].fd != server.getFd())
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
    // close(server.getFd());
    // we handle this case in Socker class. remove here later.
    return 0;
}
