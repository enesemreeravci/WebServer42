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


void Utils::closeFd(int& fd)
{
    if(fd != -1)
    {
        close(fd);
        fd = -1;
    }
    
}

//extract the directory from the given path
std::string Utils::extractDirectory(const std::string& path)
{
    size_t lastSlash = path.find_last_of('/');

    if(lastSlash == std::string::npos)
        return "";
    
    if(lastSlash == 0) // if pos of lastslah equals 0
        return "/";
    
    return path.substr(0, lastSlash); 

}

// extract the filename from the given path
std::string Utils::extractFilename(const std::string& path)
{
    size_t lastSlash = path.find_last_of('/');

    if(lastSlash == std::string::npos)
        return path;
    
    return path.substr(lastSlash + 1);
}