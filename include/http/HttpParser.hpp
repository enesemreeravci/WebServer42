#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include <string>
# include <algorithm>
# include <cctype>
# include <exception>
# include <iostream>
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

class HttpParser{
	private:
		size_t _str_pos;

		bool isValidMethod(const std::string& method) const;
		bool isValidTarget(const std::string& target) const;
		bool isValidVersion(const std::string& version) const;

		bool isToken(const unsigned char& c) const;
		bool isDelimeter(const unsigned char& c) const;
		bool isVchar(const unsigned char& c) const;
		bool isHttpWhitespace(const unsigned char& c) const;

		bool extractMethod(const std::string& rawData, HttpRequest&  req);
		bool extractTarget(const std::string& rawData, HttpRequest& req);
		bool extractVersion(const std::string& rawData, HttpRequest& req);

		bool extractFieldName(const std::string& rawData, std::string& field_name);
		bool extractFieldValue(const std::string& rawData, std::list<std::string>& field_value);
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