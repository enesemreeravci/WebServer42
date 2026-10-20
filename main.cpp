#include "sockets.hpp"

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // creating a socket does not meaning we are listening for connections yet.
    if(server_fd == -1)
    {
        perror("socket");
        return 1;
    
    }
    std::cout << 
            "Socket created successfully!\n"
            << "Socket's fd = " << server_fd << std::endl;

    // htons (host to network short)
    // htonl (host to network long)
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET; // tells the computer we are using standart ipv4
    addr.sin_port = htons(8080); // sets the port number to 8080
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //sets the ip address to 127.0.0.1 which means "localhost"
    
    struct sockaddr* generic_addr = reinterpret_cast<struct sockaddr *>(&addr); // bind() expects a pointer to sockaddr
    std::cout << "Address succesfully intialized" << std::endl;

    if(bind(server_fd, generic_addr, sizeof(addr)) < 0 )
    {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }
    std::cout << "Socket bound to 127.0.0.1:8080" << std::endl;

    if(listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }
    std::cout << "Server is listening on port 8080" << std::endl;

    int cliend_fd = accept(server_fd, NULL, NULL);
    if(cliend_fd == -1)
    {
        perror("Accept failed");
        return 1;
    }

    std::cout << "Accept connected to server successgully\n" << 
                "Client's fd = " << cliend_fd << std::endl;

    close(server_fd);
    close(cliend_fd);
}