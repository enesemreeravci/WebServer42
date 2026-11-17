/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akonstan <akonstan@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/17 17:23:31 by akonstan          #+#    #+#             */
/*   Updated: 2026/11/17 17:34:30 by akonstan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP
# include <string>
# include "HttpMessage.hpp"

class HttpResponse: public HttpMessage{
	private:
		std::string _version;
		std::string _status_code;
		std::string _reason_phrase;
	public:
		HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse& operator=(const HttpResponse& other);
		~HttpResponse();

		const std::string& getVersion() const;
		const std::string& getStatusCode() const;
		const std::string& getReasonPhrase() const;

		
};

#endif
