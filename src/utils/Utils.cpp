#include "../../include/utils/Utils.hpp"

bool Utils::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL); // get flags
    if(flags == -1)
        return false;
    int result = fcntl(fd, F_SETFL, flags | O_NONBLOCK); // old flags + nonblock
    if(result == -1)
        return false;
    return true;
}



