#include "../../include/config/ServerConfig.hpp"

LocationConfig::LocationConfig() : path(""), root(""), index(""), autoindex(false), uploadStore(""), redirectCode(0)
{

}

ServerConfig::ServerConfig() : host("0.0.0.0"), port(8080), root("./www"), index("index.html"), clientMaxBodySize(1048576)
{

}