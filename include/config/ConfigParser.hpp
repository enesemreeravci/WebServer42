#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP


#include "ServerConfig.hpp"
#include <string>
#include <vector>


class ConfigParser
{
    private:
        std::vector<std::string> tokenize(const std::string& content);

        std::vector<ServerConfig> parseFile(const std::string& filePath);
        
    public:
        ConfigParser();
        ~ConfigParser();
        std::vector<ServerConfig> parseFile(const std::string& filePath);

};

#endif