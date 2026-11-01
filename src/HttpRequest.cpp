#include "../include/HttpRequest.hpp"

// we dont need to initilaze string or map
// both types have default constructors that automatically create empty objects
HttpRequest::HttpRequest() {}

HttpRequest::~HttpRequest() {}

const std::string& HttpRequest::getMethod() const { return _method; }
const std::string& HttpRequest::getTarget() const { return _target; }
const std::string& HttpRequest::getVersion() const { return _version; }
const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return _headers; }
const std::string& HttpRequest::getBody() const { return _body; }

void HttpRequest::setMethod(const std::string& method)
{
        _method = method;
}

void HttpRequest::setTarget(const std::string& target)
{
    _target = target;
}

void HttpRequest::setVersion(const std::string& version)
{
    _version = version;
}
