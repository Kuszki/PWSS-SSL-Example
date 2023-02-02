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

#include "server.hpp"

Server::Server(void)
{
	m_method = TLS_server_method(); // Wybór metody SSL
}

Server::~Server(void) {} // Destruktor dla zachowania porządku

Wrapper::error Server::start(const std::string& host,
			    const uint16_t port,
			    const int queue)
{
	static const int yes = 1; // Zmienna do ustawienia opcji `SO_REUSEADDR`
	if (m_sock) Wrapper::close(); // Zatrzymaj serwer, jeśli jest aktywny

	// Stwórz socket - IPv4, TCP
	int sock = ::socket(PF_INET, SOCK_STREAM, 0);

	// W przypadku błędu - zakończ metodę
	if (sock == -1) return error::create_socket_fail;

	error last_err = error::no_error; // Ostatni błąd
	sockaddr_in sin; // Struktura opisująca adres
	bool ok = true; // Informacja o powodzeniu operacji

	// Uzupełnij strukturę adresu
	memset(&sin, 0, sizeof(sin));
	sin.sin_port = htons(port);
	sin.sin_family = AF_INET;

	// Konwertuj adres z łańcucha do liczby
	if (ok)
	{
		ok = ::inet_pton(AF_INET, host.c_str(), &(sin.sin_addr)) == 1;
		if (!ok) last_err = error::pton_call_error;
	}

	// Ustaw opcję ponownego użycia adresu
	if (ok)
	{
		ok = ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == 0;
		if (!ok) last_err = error::sockopt_call_error;
	}

	if (ok)
	{
		ok = ::bind(sock, (sockaddr*) &sin, sizeof(sin)) == 0;
		if (!ok) last_err = error::bind_call_error;
	}

	if (ok)
	{
		ok = ::listen(sock, queue) == 0;
		if (!ok) last_err = error::listen_call_error;
	}

	// Zapisz deskryptor gniazda jeśli wszystko jest OK
	if (!ok) { ::close(sock); return last_err; }
	else m_sockets.push_back({ m_sock = sock, POLLIN, 0 });

	return error::no_error; // Zwróć powodzenie operacji
}

Wrapper::error Server::send(const int sock,
			   const std::vector<char>& data)
{
	// Jeśli klient istnieje - wyślij dane
	if (!m_clients.contains(sock)) return error::no_client_found;
	else return m_clients[sock].send(data);
}

Wrapper::error Server::send(const int sock, const std::string& data)
{
	// Jeśli klient istnieje - wyślij dane
	if (!m_clients.contains(sock)) return error::no_client_found;
	else return m_clients[sock].send(data);
}

Wrapper::error Server::recv(const int sock, std::vector<char>& data, size_t size, bool append)
{
	// Jeśli klient istnieje - odbierz dane
	if (!m_clients.contains(sock)) return error::no_client_found;
	else return m_clients[sock].recv(data, size, append);
}

Wrapper::error Server::recv(const int sock, std::string& data, size_t size, bool append)
{
	// Jeśli klient istnieje - odbierz dane
	if (!m_clients.contains(sock)) return error::no_client_found;
	else return m_clients[sock].recv(data, size, append);
}

Wrapper::error Server::close(int sock)
{
	// Jeśli klient nie istnieje - zakończ
	if (!m_clients.contains(sock)) return error::no_client_found;

	auto st = m_sockets.begin() + 1; // Początek klientów
	auto end = m_sockets.end(); // Koniec kontenera

	// Wyszukaj strukturę zamykanego gniazda
	auto it = std::find_if(st, end,
	[sock] (const auto& i) -> bool
	{
		return i.fd == sock;
	});

	m_clients.erase(sock); // Usuń obiekt klienta
	m_sockets.erase(it); // Usuń gniazdo z listy

	return error::no_error; // Poinformuj o sukcesie
}

Wrapper::error Server::loop(std::set<int>& read,
			   std::set<int>& write,
			   std::set<int>& open,
			   const time_t timeout)
{
	// Sprawdź wszystkie gniazda pod kątem ich obsługi
	int count = poll(m_sockets.data(), m_sockets.size(), timeout);

	if (count < 0) return error::poll_call_error; // Gdy błąd - zakończ
	else if (count > 0) // W przeciwnym razie obsługuj połączenia
	{
		// Jeśli jest nowe połączenie do serwera
		if (m_sockets.front().revents & POLLIN)
		{
			// Jeśli uda się wynegocjować połączenie - dodaj do nowych połączeń
			if (accept() == error::no_error) open.insert(m_sockets.back().fd);
		}

		// Obsłuż wszystkie gniazda klientów
		for (auto i = m_sockets.begin()+1; i != m_sockets.end();)
		{
			// W przypadku błędu/rozłączenia zamknij klienta
			if (i->revents & (POLLHUP | POLLERR))
			{
				m_clients.erase(i->fd); // Usuń obiekt klienta
				i = m_sockets.erase(i); // Usuń gniazdo z listy
			}
			else
			{
				if (i->revents & POLLIN) read.insert(i->fd); // Gotowy do odczytu
				if (i->revents & POLLOUT) write.insert(i->fd); // Gotowy do zapisu

				++i; // Przejdź do kolejnego klienta
			}
		}
	}

	return error::no_error; // Zwróć powodzenie
}

Wrapper::error Server::flag(int sock, short flags, bool mode)
{
	auto st = m_sockets.begin() + 1; // Początek klientów
	auto end = m_sockets.end(); // Koniec kontenera

	// Wyszukaj strukturę zamykanego gniazda
	auto it = std::find_if(st, end,
	[sock] (const auto& i) -> bool
	{
		return i.fd == sock;
	});

	// Gdy nie znaleziono - zwróć błąd
	if (it == end) return error::no_client_found;

	// Modyfikuj wybrane flagi zgodnie z trybem
	if (mode) it->events |= flags;
	else it->events &= ~flags;

	return error::no_error; // Zwróć powodzenie
}

std::string Server::name(int sock) const
{
	// Jeśli klient istnieje - zwróć jego nazwę
	if (!m_clients.contains(sock)) return std::string();
	else return m_clients.at(sock).name();
}

std::string Server::cipher(int sock) const
{
	// Jeśli klient istnieje - zwróć jego nazwę
	if (!m_clients.contains(sock)) return std::string();
	else return m_clients.at(sock).cipher();
}

Wrapper::error Server::accept(void)
{
	if (!m_sock) return error::no_active_socket; // Jeśli serwer nie jest aktywny - zwróć błąd

	sockaddr_in sin; // Struktura pomocnicza na adres
	socklen_t size = sizeof(sin); // Długość adresu

	// Akceptuj nowe połączenie do serwera
	int sock = ::accept(m_sock, (sockaddr*) &sin, &size);
	SSL* ssl = nullptr; // Obiekt SSL

	// Jeśli nie udało się zaakceptować połączenia - zakończ
	if (sock == -1) return error::accept_call_error;
	else if (m_ctx) // Jeśli utworzono kontekst SSL
	{
		ssl = SSL_new(m_ctx); // Utwórz obiekt SSL

		// Jeśli nie udało się utworzyć obiektu SSL
		if (ssl == nullptr)
		{
			::close(sock); // Zamknij wadliwe połączenie

			return error::create_sslobj_fail; // Zwróć informację o błędzie
		}

		// Spróbuj nawiązać bezpiecznie połączenie z klientem
		if (SSL_set_fd(ssl, sock) != 1 || // Powiąż gniazdo z obiektem SSL
		    SSL_accept(ssl) != 1) // Wynegocjuj warunki połączenia SSL
		{
			SSL_free(ssl); // Zwolnij obiekt jeśli się nie udało
			::close(sock); // Zamknij wadliwe połączenie

			return error::sslwrap_call_error; // Zwróć informację o błędzie
		}
	}

	// Dodaj gniazdo do listy klientów i utwórz obiekt klienta
	m_sockets.push_back({ sock, POLLIN | POLLHUP, 0 });
	m_clients.emplace(std::piecewise_construct,
				   std::forward_as_tuple(sock),
				   std::forward_as_tuple(m_ctx, ssl, sock));

	return error::no_error; // Zwróć informacje o powodzeniu
}

std::set<int> Server::list(void) const
{
	std::set<int> list; // Lista wszystkich klientów

	// Dodaj do zbioru identyfikatory wszystkich klientów
	for (auto i = m_sockets.cbegin()+1; i != m_sockets.cend(); ++i)
	{
		list.insert(i->fd); // Dodaj klienta do listy
	}

	return list; // Zwróć listę klientów
}
