#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <vector>

namespace Utils
{
    bool setNonBlocking(int fd);

    // add other helper here for http and configs and routers
}

#endif