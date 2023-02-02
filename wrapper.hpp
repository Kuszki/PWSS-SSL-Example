/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Open SSL chat example                                                  *
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

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>

#include "version.hpp"

//! Klasa owijająca gniazdo SSL.
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
			no_error = 0, //!< Brak błędu.

			no_active_socket, //!< Brak otwartego gniazda.
			is_active_socket, //!< Istnieje otwarte gniazdo.

			create_context_fail, //!< Błąd tworzenia kontekstu SSL.
			create_socket_fail, //!< Błąd tworzenia gniazda.
			create_sslobj_fail, //!< Błąd tworzenia obiektu SSL.

			pton_call_error, //!< Błąd w wywołaniu pton.
			sockopt_call_error, //!< Błąd w ustawianiu opcji gniazda.
			bind_call_error, //!< Błąd w wywołaniu bind.
			listen_call_error, //!< Błąd w wywołaniu listen.
			poll_call_error, //!< Błąd w wywołaniu poll.
			accept_call_error, //!< Błąd w wywołaniu accept.
			sslwrap_call_error, //!< Błąd podczas owijania gniazda.
			addrinfo_call_error, //!< Błąd podczas tłumaczenia adresu.

			send_call_error, //!< Błąd podczas wywołania send.
			recv_call_error, //!< Błąd podczas wywołania recv.

			no_client_found, //!< Nie znaleziono zadanego klienta.
			no_server_found //!< Brak połączenia z serwerem.
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
		std::string cipher(void) const; //!< Pobranie nazwy szyfru.

		Wrapper& operator= (const Wrapper&) = delete; //!< Operator przypisania (usunięty).
		Wrapper& operator= (Wrapper&&) = delete; //!< Operator przeniesienia (usunięty).

		static bool is_ok(Wrapper::error err /*!< [in] Kod błędu. */); //!< Sprawdzenie, czy nie wystąpił błąd.
		static std::string version(bool sh = false /*!< [in] Format */); //!< Pobranie numeru wersji.
		static std::string compiler(void); //!< Pobranie nazwy i wersji kompilatora.

};

#endif // WRAPPER_HPP
