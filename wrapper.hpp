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
#include <map>
#include <set>

//! Klasa owijajaca gniazdo SSL.
class Wrapper
{

	protected:

		const SSL_METHOD* m_method = nullptr; //!< Metoda SSL.

		SSL_CTX* m_ctx = nullptr; //!< Kontekst SSL.
		SSL* m_ssl = nullptr; //!< Obiekt SSL.

		bool m_shared = false; //!< Flaga współdzielenia kontekstu.
		int m_sock = 0; //!< Identyfikator gniazda.

	protected:

		Wrapper(const Wrapper&) = delete; //! Konstruktor kopiujący - usunięty.
		Wrapper(Wrapper&& wrapper); //! Konstruktor przenoszący.
		Wrapper(void); //!< Konstruktor domyślny.

	public:

		enum class error : int
		{
			no_error = 0,

			no_active_socket,
			is_active_socket,

			create_context_fail,
			create_socket_fail,
			create_sslobj_fail,

			pton_call_error,
			sockopt_call_error,
			bind_call_error,
			listen_call_error,
			poll_call_error,
			accept_call_error,
			sslwrap_call_error,
			addrinfo_call_error,

			send_call_error,
			recv_call_error,

			no_client_found,
			no_server_found
		};

		virtual ~Wrapper(void); //!< Destruktor.

		//! Inicjalizacja kontekstu SSL nowym kontekstem.
		error init(const std::string& cert, /*!< [out] Ścieżka pliku z certyfikatem. */
		           const std::string& key, /*!< [out] Ścieżka pliku z kluczem prywatnym. */
		           const std::string& ca /*!< [out] Ścieżka pliku z certyfikatem głównym. */);

		//! Inicjalizacja kontekstu SSL kontekstem współdzielonym.
		error init(SSL_CTX* ctx /*!< Kontekst SSL. */);

		error close(void); //!< Zamkniecie gniazda.

		int sock(void) const; //!< Pobranie identyfikatora gniazda.

		std::string name(void) const; //!< Pobranie nazwy hosta.

		Wrapper& operator= (const Wrapper&) = delete; //!< Operator przypisania (usuniety).
		Wrapper& operator= (Wrapper&&) = delete; //!< Operator przeniesienia (usuniety).

		static bool is_ok(Wrapper::error err);

};

#endif // WRAPPER_HPP
