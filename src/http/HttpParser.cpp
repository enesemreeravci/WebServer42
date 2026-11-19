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

bool HttpParser::isToken(const unsigned char& c) const
{
    return ((c >= '0' && c <= '9')
			|| (c >= 'A' && c <= 'Z')
        	|| (c >= 'a' && c <= 'z')
        	|| c == '!' || c == '#' || c == '$' || c == '%'
        	|| c == '&' || c == '\'' || c == '*' || c == '+'
        	|| c == '-' || c == '.' || c == '^' || c == '_'
        	|| c == '`' || c == '|' || c == '~');
}

bool HttpParser::isDelimeter(const unsigned char& c) const{
	return (c == '(' || c == ')' || c == ',' || c == '/' 
			|| c == ':' || c == ';' || c == '<' || c == '='
			|| c == '>' || c == '?' || c == '@' || c == '['
			|| c == '\\' || c == ']' || c == '{' || c == '}');
}

//Checks if it is a valid character for field-value 
bool HttpParser::isVchar(const unsigned char& c) const{
	return ((c >= 32 || c == '\t') && c != 127);
}

bool HttpParser::isHttpWhitespace(const unsigned char& c) const{
	return (c == ' ' || c == '\t');
}

HttpParser::HttpParser()
	: _str_pos(0)
{}

HttpParser::~HttpParser(){

}

bool HttpParser::extractMethod(const std::string& rawData, HttpRequest&  req){
	if (rawData[_str_pos] == ' ' || std::isprint(rawData[_str_pos]) == 0)
		return false;
	
	size_t end = rawData.find(' ', _str_pos);
	if (end == std::string::npos)
		return false;
	
	std::string method = rawData.substr(_str_pos, end - _str_pos);
	_str_pos = end + 1;
	if (isValidMethod(method) == true){
		req.setMethod(method);
		return true;
	}

	return false;
}

bool HttpParser::extractTarget(const std::string& rawData, HttpRequest& req){
	if (rawData[_str_pos] == ' ' || std::isprint(rawData[_str_pos]) == 0)
		return false;

	size_t end = rawData.find(' ', _str_pos);
	if (end == std::string::npos)
		return false;
	
	std::string target = rawData.substr(_str_pos, end - _str_pos);
	_str_pos = end + 1;
	if (isValidTarget(target) == true){
		req.setTarget(target);
		return true;
	}

	return false;
}

bool HttpParser::extractVersion(const std::string& rawData, HttpRequest& req){
	if (rawData[_str_pos] == ' ' || std::isprint(rawData[_str_pos]) == 0)
		return false;

	size_t end = rawData.find("\r\n", _str_pos);
	if (end == std::string::npos)
		return false;
	
	std::string version = rawData.substr(_str_pos, end - _str_pos);
	_str_pos = end + 2;
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

bool HttpParser::extractFieldName(const std::string& rawData, std::string& field_name){
	if (_str_pos >= rawData.size())
		return false;

	size_t colon = rawData.find(':', _str_pos);
	if (colon == std::string::npos || colon == _str_pos)
		return false;

	for (size_t cur = _str_pos; cur < colon; ++cur){
		if (isToken(static_cast<unsigned char>(rawData[cur])) == false)
			return false;
	}

	field_name = rawData.substr(_str_pos, colon - _str_pos);
	_str_pos = colon + 1;
	for (char& c : field_name){
		if (c >= 'a' && c <= 'z')
			c = static_cast<char>(c + 'A' - 'a');
	}
	return true;
}

bool HttpParser::extractFieldValue(const std::string& rawData, std::list<std::string>& field_value){
	field_value.clear();

	if (_str_pos >= rawData.size())
		return false;

	size_t field_end = rawData.find("\r\n", _str_pos);
	if (field_end == std::string::npos)
		return false;

	size_t start = _str_pos;
	bool in_quotes = false;
	bool escaped = false;

	for (size_t cur = _str_pos; cur < field_end; ++cur){
		unsigned char c = static_cast<unsigned char>(rawData[cur]);

		if (isVchar(c) == false)
			return false;
		if (escaped == true){
			escaped = false;
			continue;
		}
		if (in_quotes){
			if (c == '\\')
				escaped = true;
			else if (c == '"')
				in_quotes = false;
			continue;
		}

		switch (c){
			case '"':
				in_quotes = true;
				continue;
			case ',':{
				size_t end = cur;

				while (start < end && isHttpWhitespace(static_cast<unsigned char>(rawData[start])))
					++start;
				while (end > start && isHttpWhitespace(static_cast<unsigned char>(rawData[end - 1])))
					--end;
				field_value.push_back(rawData.substr(start, end - start));
				start = cur + 1;
				break;
			}
			default:
				break;
		}
	}

	if (in_quotes || escaped)
			return false;
		
	size_t end = field_end;

	while (start < end && isHttpWhitespace(static_cast<unsigned char>(rawData[start])))
		++start;
	while (end > start && isHttpWhitespace(static_cast<unsigned char>(rawData[end - 1])))
		--end;
	field_value.push_back(rawData.substr(start, end - start));
	_str_pos = field_end + 2;

	return true;
}

//Need to add header field length limits, to cover vulnerability from insanely long header attacks
bool HttpParser::parseHeader(const std::string& rawData, HttpMessage& msg){
	if (_str_pos >= rawData.size())
		return false;

	size_t header_end = rawData.find("\r\n", _str_pos);
	if (header_end == std::string::npos)
		return false;
	
	while (_str_pos != header_end){
		std::string field_name;
		std::list<std::string> field_value;

		extractFieldName(rawData, field_name);
		extractFieldValue(rawData, field_value);
		msg.addHeader(field_name, field_value);
	}
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