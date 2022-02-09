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

#ifndef SERVER_HPP
#define SERVER_HPP

#include "wrapper.hpp"
#include "client.hpp"

#include <string>
#include <vector>
#include <map>
#include <set>

class Server : public Wrapper
{

	protected:

		std::map<int, Client> m_clients;
		std::vector<pollfd> m_sockets;

	public:

		Server(Server&&) = delete;
		Server(void);

		virtual ~Server(void);

		bool start(const std::string& host = "127.0.0.1",
		           const uint16_t port = 8080,
		           const int queue = 10);

		bool send(const int sock,
		          const std::vector<char>& data);

		bool send(const int sock,
		          const std::string& data);

		bool recv(const int sock,
		          std::vector<char>& data,
		          size_t size = 1024);

		bool recv(const int sock,
		          std::string& data,
		          size_t size = 1024);

		bool close(int sock);

		bool accept(void);

		bool loop(std::set<int>& read,
		          std::set<int>& write,
		          std::set<int>& open,
		          const time_t timeout = -1);

		std::string name(int sock) const;

		std::set<int> list(void) const;

};

#endif // SERVER_HPP
