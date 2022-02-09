/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  {description}                                                          *
 *  Copyright (C) 2022  Łukasz "Kuszki" Dróżdż  lukasz.kuszki@gmail.com    *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the  Free Software Foundation, either  version 3 of the  License, or   *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This  program  is  distributed  in the hope  that it will be useful,   *
 *  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
 *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have  received a copy  of the  GNU General Public License   *
 *  along with this program. If not, see http://www.gnu.org/licenses/.     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "wrapper.hpp"

#include <string>
#include <vector>

class Client : public Wrapper
{

	public:

		Client(Client&& client) = default;
		Client(SSL_CTX* ctx,
		       SSL* ssl,
		       int sock);
		Client(void);

		virtual ~Client(void);

		bool open(const std::string& host = "localhost",
		          const uint16_t port = 8080);

		bool send(const std::vector<char>& data);

		bool send(const std::string& data);

		bool recv(std::vector<char>& data,
		          size_t size = 1024);

		bool recv(std::string& data,
		          size_t size = 1024);

};

#endif // CLIENT_HPP
