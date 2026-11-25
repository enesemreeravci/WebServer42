#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>
#include <vector>
#include <map>

// location /path {...} 
struct LocationConfig
{   
    std::string path; // "/"  "/cgi-bin"
    std::string root;
    std::string index;
    std::vector<std::string> allowedMethods; // get post delete
    bool autoindex; // default false;
    std::string uploadStore; 
    std::string cgiExt; // .py
    std::string cgiPass; // /usr/bin/python3
    int redirectCode;
    std::string redirectUrl;

    LocationConfig();

};

struct ServerConfig
{
    std::string host;
    int port;
    std::vector<std::string> serverNames;
    std::string root;
    std::string index;
    size_t clientMaxBodySize;
    std::map<int, std::string> errorPages; // 404 -> "/errors/404.html"
    std::vector<LocationConfig> locations;

    ServerConfig();

};

#endif