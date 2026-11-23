#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <vector>
#include <string>
#include <sstream>


namespace Utils
{
    bool setNonBlocking(int fd);

    void closeFd(int& fd); 

    std::string extractDirectory(const std::string& path);
    std::string extractFilename(const std::string& path);

    // add other helper here for http and configs and routers
}

#endif