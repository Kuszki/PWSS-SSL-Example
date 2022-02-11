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

		virtual ~Wrapper(void); //!< Destruktor.

		//! Inicjalizacja kontekstu SSL nowym kontekstem.
		bool init(const std::string& cert, /*!< [out] Ścieżka pliku z certyfikatem. */
		          const std::string& key, /*!< [out] Ścieżka pliku z kluczem prywatnym. */
		          const std::string& ca /*!< [out] Ścieżka pliku z certyfikatem głównym. */);

		//! Inicjalizacja kontekstu SSL kontekstem współdzielonym.
		bool init(SSL_CTX* ctx /*!< Kontekst SSL. */);

		bool close(void); //!< Zamkniecie gniazda.

		int sock(void) const; //!< Pobranie identyfikatora gniazda.

		std::string name(void) const; //!< Pobranie nazwy hosta.

		Wrapper& operator= (const Wrapper&) = delete; //!< Operator przypisania (usuniety).
		Wrapper& operator= (Wrapper&&) = delete; //!< Operator przeniesienia (usuniety).

};

#endif // WRAPPER_HPP
