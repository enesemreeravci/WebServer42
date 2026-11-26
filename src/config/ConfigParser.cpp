#include "../../include/config/ConfigParser.hpp"

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}


std::vector<std::string> ConfigParser::tokenize(const std::string& content)
{
    std::vector<std::string> tokens;
    std::string current;

    for(size_t i = 0; i < content.size(); ++i)
    {
        if(content[i] == '#')
        {
            if(!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
            while(i < content.size() && content[i] != '\n')
                ++i;
        }
        else if(std::isspace(static_cast<unsigned char>(content[i])))
        {
            if(!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
        }
        else if(content[i] == '{' || content[i] == '}' || content[i] == ';')
        {
            if(!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(std::string(1, content[i]));
        }
        else
            current += content[i];
    }
    if(!current.empty())
        tokens.push_back(current);
    return tokens;
}


std::vector<ServerConfig> ConfigParser::parseFile(const std::string& filePath)
{
    std::ifstream file(filePath.c_str());

    if(!file.is_open())
        throw std::runtime_error("Could not open config file: " + filePath);
    
    std::stringstream buffer;

    buffer << file.rdbuf();

    std::string content = buffer.str();

    std::vector<std::string> tokens = tokenize(content);

    // after finishin server fix here
    return std::vector<ServerConfig>();
}

void ConfigParser::expectToken(const std::vector<std::string>& tokens, size_t& index, const std::string& expected)
{
    if(index >= tokens.size())
    {
        throw std::runtime_error("Config error: unexpected end of file, expected '" + expected + "'");
    }

    if(tokens[index] != expected)
    {
        throw std::runtime_error("Config error: expcected '" + expected + "' , got '" + tokens[index]  + "'");
    }

    ++index;
}

std::vector<std::string> ConfigParser::parseDirectiveArgs(const std::vector<std::string>& tokens, size_t& index)
{
    std::vector<std::string> args;
    while(index < tokens.size() && tokens[index] != ";")
    {
        if(tokens[index] == "{" || tokens[index] == "}")
        {
            throw std::runtime_error("Config error: unexcpected '" + tokens[index] + "' before '; '");
        }
        args.push_back(tokens[index]);
        ++index;
    }
    expectToken(tokens, index, ";");

    if(args.empty())
    {
        throw std::runtime_error("Confif error: directive requires at least one argument");
    }
    return args;
}