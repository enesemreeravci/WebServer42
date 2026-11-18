#include "../../include/http/HttpParser.hpp"

/*
rawData:
"GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"

requestLine:
"GET /index.html HTTP/1.1"
*/

bool HttpParser::isValidMethod(const std::string& method) const{
	return (method == "GET" || method == "POST" || method == "DELETE");
}

bool HttpParser::isValidTarget(const std::string& target) const
{
	if(target.empty())
		return false;
	
	if(target[0] != '/') // target starts always with '/'
		return false;
	
		// no need for this, remove later!
		// check path traversel
	if(target.find("..") != std::string::npos)
		return false;

	for(size_t i = 0; i < target.size(); i++)
	{
		char c = target[i];

		if(c <= 32 || c == 127)
			return false;

		// check characteres that must not be in target
		if(c == '"' || c == '\'' || c == '<' || c == '>')
			return false;
	}

	// this can be remove later, i just wanted to limit the target lentgh
	if(target.length() > 2048)
		return false;
	
	return true;
}

bool HttpParser::isValidVersion(const std::string& version) const
{
	return (version == "HTTP/1.1" || version == "HTTP/1.0");
}

bool HttpParser::isValidRequest(const std::string& method,
								const std::string& target,
								const std::string& version) const
{
	return (isValidMethod(method) && isValidTarget(target) && isValidVersion(version));   
}

HttpParser::HttpParser()
	: str_pos(0)
{}

HttpParser::~HttpParser(){

}

bool HttpParser::extractMethod(const std::string& rawData, HttpRequest&  req){
	if (rawData[str_pos] == ' ' || std::isprint(rawData[str_pos]) == 0)
		return false;
	
	size_t end = rawData.find(' ', str_pos);
	if (end == std::string::npos)
		return false;
	
	std::string method = rawData.substr(str_pos, end - str_pos);
	str_pos = end + 1;
	if (isValidMethod(method) == true){
		req.setMethod(method);
		return true;
	}

	return false;
}

bool HttpParser::extractTarget(const std::string& rawData, HttpRequest& req){
	if (rawData[str_pos] == ' ' || std::isprint(rawData[str_pos]) == 0)
		return false;

	size_t end = rawData.find(' ', str_pos);
	if (end == std::string::npos)
		return false;
	
	std::string target = rawData.substr(str_pos, end - str_pos);
	str_pos = end + 1;
	if (isValidTarget(target) == true){
		req.setTarget(target);
		return true;
	}

	return false;
}

bool HttpParser::extractVersion(const std::string& rawData, HttpRequest& req){
	if (rawData[str_pos] == ' ' || std::isprint(rawData[str_pos]) == 0)
		return false;

	size_t end = rawData.find("\r\n", str_pos);
	if (end == std::string::npos)
		return false;
	
	std::string version = rawData.substr(str_pos, end - str_pos);
	str_pos = end + 2;
	if (isValidVersion(version) == true){
		req.setVersion(version);
		return true;
	}

	return false;
}


bool HttpParser::parseRequestLine(const std::string& rawData, HttpRequest& req){
	

	if (extractMethod(rawData, req) && extractTarget(rawData, req) && extractVersion(rawData, req)){
		req.setStartLine();
		return true;
	}
	
	return false;
}

std::string& HttpParser::extractKey(const std::string& rawData){
	if (std::isspace(rawData[str_pos]) != 0 || std::isalpha(rawData[str_pos]) == 0){
		return "";
	}

	size_t colon = str_pos + 1;

	while (colon != std::string::npos && rawData[colon] != ':'){
		if (!std::isalnum(rawData[colon]) && rawData[colon] != '-'){
			return "";
		}
		++colon;
	}

	if (colon == std::string::npos){
		return "";
	}

	std::string key = rawData.substr(str_pos, colon - str_pos);
	str_pos = colon + 1;
	std::transform(key.begin(), key.end(), key.begin(), std::toupper);

	return key;
}

list<std::string>& HttpParser::exctractValue(const std::string& rawData){
	while (std::isspace(rawData[str_pos]) && str_pos != std::string::npos)
		str_pos++;

	if (std::)
	size_t end = str_pos;
}

bool HttpParser::parseHeader(const std::string& rawData, HttpRequest& req){
	if (rawData == NULL || rawData == "")
		return false;
	
	size_t end = rawData.find("\r\n", str_pos);
	if (end == std::string::npos)
		return false;


}

/*bool HttpParser::parseHeader(const std::string& headerLine, HttpRequest& request)
{
	size_t colonPos = headerLine.find(':');
	// get the first : postion and validate it
	if(colonPos == 0 || colonPos == std::string::npos)
		return false;
	std::string name = headerLine.substr(0, colonPos);
	std::string value = headerLine.substr(colonPos + 1);
	while(!value.empty() && (value[0] == ' ' || value[0] == '\t'))
	{
		value.erase(0, 1);
	}
	request.addHeader(name, value);
	return true;
}*/

bool HttpParser::parseHeaders(const std::string& rawData, HttpRequest& request)
{
	size_t requestLineEnd = rawData.find("\r\n");
	if(requestLineEnd == std::string::npos)
		return false;
	// requestLine: "GET /index.html HTTP/1.1"
	// we kinda skipping the request line and getting the position of header
	size_t headersStart = requestLineEnd + 2; // it points to H in "Host"

	size_t headersEnd = rawData.find("\r\n\r\n"); 
	if(headersEnd == std::string::npos)
		return false;
	
	// means there are no header lines to parse
	// return true because the empty header section has been parsed successfully
	if(headersEnd == requestLineEnd)
		return true;
	
	if(headersStart > headersEnd)
		return false;
	while (headersStart < headersEnd)
	{
		size_t nextHeaderEnd = rawData.find("\r\n", headersStart); 
		// searched from H to end of that line
		// so it points to the \r immediately after localhost
			
		if(nextHeaderEnd == std::string::npos)
			return false;
		if(nextHeaderEnd > headersEnd) 
			return false;
		const std::string headerLine = rawData.substr(headersStart, nextHeaderEnd - headersStart);
		
		if(!parseHeader(headerLine, request))
			return false;
		headersStart = nextHeaderEnd + 2;
	}
	return true;
}                       

// bool HttpParser::parseBody(const std::string& rawData, HttpRequest& request)
// {
//     size_t headersEnd = rawData.find("\r\n\r\n");
//     if(headersEnd == std::string::npos)
//         return false;
	
//     size_t bodyStart = headersEnd + 4;

//     size_t availableBodyBytes = rawData.size() - bodyStart;
//         return true;
// }