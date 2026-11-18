/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMessage.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akonstan <akonstan@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/17 15:48:14 by akonstan          #+#    #+#             */
/*   Updated: 2026/11/18 16:19:37 by akonstan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/HttpMessage.hpp"

HttpMessage::HttpMessage()
	: _start_line(NULL)
	, _header(NULL)
	, _body(NULL)
{}

HttpMessage::HttpMessage(const HttpMessage& other)
	: _start_line(other._start_line)
	, _header(other._header)
	, _body(other.body)
{}

HttpMessage& HttpMessage::operator=(const HttpMessage& other){
	if (this != &other){
		this->_start_line = other._start_line;
		this->_header = other._header;
		this->_body = other._body;
	}
	return (*this);
}

HttpMessage::~HttpMessage(){}

const std::string& HttpMessage::getStartLine() const{
	return _start_line;
}

const std::map<std::string, std::string>& HttpMessage::getHeader() const{
	return _header;
}

const std::string& HttpMessage::getBody() const{
	return _body;
}

void HttpMessage::addHeader(const std::string& key, const list<std::string>& value){
    _headers[key] = value;
}
