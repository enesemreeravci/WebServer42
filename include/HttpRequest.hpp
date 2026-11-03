#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <iostream>
#include <string>
#include <map>

class HttpRequest
{
    private:
        std::string _method;
        std::string _target;
        std::string _version; 
        std::map<std::string, std::string> _headers;
        std::string _body;
        
    public:
        HttpRequest();
        ~HttpRequest();    

        //getters
        const std::string& getMethod() const;
        const std::string& getTarget() const;
        const std::string& getVersion() const;
        const std::map<std::string, std::string>& getHeaders() const;
        const std::string& getBody() const;
        
        //setters
        void setMethod(const std::string& method);
        void setTarget(const std::string& target);
        void setVersion(const std::string& version);

        //functions
        void addHeader(const std::string& name, const std::string& value);
};

#endif