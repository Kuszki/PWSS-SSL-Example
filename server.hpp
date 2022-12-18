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

#ifndef SERVER_HPP
#define SERVER_HPP

#include "wrapper.hpp"
#include "client.hpp"

//! Klasa reprezentująca serwer.
class Server : public Wrapper
{

	protected:

		std::map<int, Client> m_clients; //!< Mapa klientów.
		std::vector<pollfd> m_sockets; //!< Lista monitorowanych gniazd.

	public:

		Server(Server&&) = default;  //! Konstruktor przenoszący - domyślny.
		Server(void); //!< Konstruktor domyślny.

		virtual ~Server(void);  //!< Destruktor.

		//! Uruchomienie serwera.
		error start(const std::string& host = "127.0.0.1", /*!< [in] Adres nasłuchiwania. */
				  const uint16_t port = 8080, /*!< [in] Numer portu. */
				  const int queue = 10 /*!< [in] Długość kolejki. */);

		//! Wysłanie wektora danych do klienta.
		error send(const int sock, /*!< [in] Identyfikator klienta. */
				 const std::vector<char>& data /*!< [in] Dane do wysłania. */);

		//! Wysłanie napisu do klienta.
		error send(const int sock, /*!< [in] Identyfikator klienta. */
				 const std::string& data /*!< [in] Dane do wysłania. */);

		error recv(const int sock, /*!< [in] Identyfikator klienta. */
				 std::vector<char>& data, /*!< [out] Bufor na dane. */
				 size_t size = 1024 /*!< [in] Maksymalna liczba danych. */);

		error recv(const int sock, /*!< [in] Identyfikator klienta. */
				 std::string& data, /*!< [out] Bufor na dane. */
				 size_t size = 1024 /*!< [in] Maksymalna liczba danych. */);

		error close(int sock /*!< [in] Identyfikator klienta. */);

		error accept(void); //!< Akceptacja nowego połączenia.

		error loop(std::set<int>& read, /*!< [out] Zbiór klientów gotowych do odczytu. */
				 std::set<int>& write, /*!< [out] Zbiór klientów gotowych do zapisu. */
				 std::set<int>& open, /*!< [out] Zbiór nowo podłączonych klientów. */
				 const time_t timeout = -1 /*!< [in] Maksymalny czas oczekiwania. */);

		error flag(int sock, /*!< [in] Identyfikator klienta. */
				 short flags, /*!< [in] Flagi do modyfikacji. */
				 bool mode /*!< [in] Operacja na flagach. */);

		std::string name(int sock /*!< [in] Identyfikator klienta. */) const;

		std::set<int> list(void) const;

};

#endif // SERVER_HPP
