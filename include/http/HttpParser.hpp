#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include <string>
# include <algorithm>
# include <cctype>
# include <exception>
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

class HttpParser{
    private:
        size_t str_pos;

        bool isValidMethod(const std::string& method) const;
        bool isValidTarget(const std::string& target) const;
        bool isValidVersion(const std::string& version) const;

        bool extractMethod(const std::string& rawData, HttpRequest&  req);
        bool extractTarget(const std::string& rawData, HttpRequest& req);
        bool extractVersion(const std::string& rawData, HttpRequest& req);

        std::string& extractKey(const std::string& rawData);
        list<std::string>& exctractValue(const std::string& rawData);
    public:
        HttpParser();
        ~HttpParser();
        bool parseRequestLine(const std::string& rawData, HttpRequest& req);
        bool parseHeader(const std::string& rawData, HttpMessage& msg);
};

/*class HttpParser
{
    private:
        bool isValidMethod(const std::string& method) const;
        bool isValidTarget(const std::string& target) const;
        bool isValidVersion(const std::string& version) const;

        // instead of calling each of them seperatly, I put them together.
        // we can invoke only this function and it will validate every cases
        bool isValidRequest(const std::string& method, 
                            const std::string& target,
                            const std::string& version) const;

    public:
        // rawData contains the HTTP bytes received from the client
        // request the existing HTTPRequest objcet that will store parsed values
        bool parseRequestLine(const std::string& rawData, HttpRequest& request);
        bool parseHeader(const std::string& headerLine, HttpRequest& request); // processes one header
        bool parseHeaders(const std::string& rawData, HttpRequest& request); // processes multiple headers
        // bool parseBody(const std::string& rawData, HttpRequest& request);

        
};*/

#endif