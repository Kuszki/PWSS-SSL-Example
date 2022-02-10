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

#ifndef WRAPPER_HPP
#define WRAPPER_HPP

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <netdb.h>

#include <string>
#include <vector>

class Wrapper
{

	protected:

		const SSL_METHOD* m_method = nullptr;

		SSL_CTX* m_ctx = nullptr;
		SSL* m_ssl = nullptr;

		bool m_shared = false;
		int m_sock = 0;

	protected:

		Wrapper(const Wrapper&) = delete;
		Wrapper(Wrapper&& wrapper);
		Wrapper(void);

	public:

		virtual ~Wrapper(void);

		bool init(const std::string& cert,
		          const std::string& key,
		          const std::string& ca);

		bool init(SSL_CTX* ctx);

		bool close(void);

		int sock(void) const;

		std::string name(void) const;

		Wrapper& operator= (const Wrapper&) = delete;
		Wrapper& operator= (Wrapper&&) = delete;

};

#endif // WRAPPER_HPP
