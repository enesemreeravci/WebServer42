#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP


#include "ServerConfig.hpp"
#include <string>
#include <vector>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

class ConfigParser
{
    private:
        void expectToken(const std::vector<std::string>& tokens, size_t& index, const std::string& expected);
        std::vector<std::string> parseDirectiveArgs(const std::vector<std::string>& tokens, size_t& index);
    public:
    ConfigParser();
    ~ConfigParser();
        std::vector<std::string> tokenize(const std::string& content);
        std::vector<ServerConfig> parseFile(const std::string& filePath);
        
    };

#endif