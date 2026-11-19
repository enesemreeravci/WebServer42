/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMessage.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akonstan <akonstan@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/17 15:12:34 by akonstan          #+#    #+#             */
/*   Updated: 2026/11/19 18:17:52 by akonstan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPMESSAGE_HPP
# define HTTPMESSAGE_HPP
# include <map>
# include <list>
# include <string>

class HttpMessage{
	protected:
		std::string	_start_line;
		std::map<std::string, std::list<std::string> > _header;
		std::string	_body;
		
		HttpMessage();
		HttpMessage(const HttpMessage& other);
	public:
		HttpMessage& operator=(const HttpMessage& other);
		virtual ~HttpMessage();

		const std::string& getStartLine() const;
		const std::map<std::string, std::string>& getHeader() const;
        const std::string& getBody() const;

		virtual void setStartLine() = 0;
		void addHeader(const std::string& key, const std::list<std::string>& value);
};
#endif