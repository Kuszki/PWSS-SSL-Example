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

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "wrapper.hpp"

//! Klasa reprezentująca klienta.
class Client : public Wrapper
{

	public:

		//! Konstruktor zawijający w obiekt.
		Client(SSL_CTX* ctx, /*!< [in] Kontekst SSL. */
			  SSL* ssl, /*!< [in] Obiekt SSL. */
			  int sock /*!< [in] Deskryptor gniazda. */);

		Client(Client&&) = default; //! Konstruktor przenoszący - domyślny.
		Client(void); //!< Konstruktor domyślny.

		virtual ~Client(void); //!< Destruktor.

		//! Nawiązanie połączenia z wybranym serwerem.
		error open(const std::string& host = "localhost" /*!< [in] Nazwa hosta. */,
				 const uint16_t port = 8080 /*!< [in] Numer portu. */);

		//! Wysłanie wektora danych.
		error send(const std::vector<char>& data /*!< [in] Dane do wysłania. */);

		//! Wysłanie napisu.
		error send(const std::string& data /*!< [in] Dane do wysłania. */);

		//! Pobranie wektora danych.
		error recv(std::vector<char>& data, /*!< [out] Bufor na dane. */
				 size_t size = 1024, /*!< [in] Maksymalna liczba danych. */
				 bool append = false /*!< [in] Dodaj dane do bufora. */);

		//! Pobranie napisu.
		error recv(std::string& data, /*!< [out] Bufor na dane. */
				 size_t size = 1024, /*!< [in] Maksymalna liczba danych. */
				 bool append = false /*!< [in] Dodaj dane do bufora. */);

};

#endif // CLIENT_HPP
