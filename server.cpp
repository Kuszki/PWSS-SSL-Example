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

#include "server.hpp"

Server::Server(void)
{
	m_method = TLS_server_method();
}

Server::~Server(void) {}

bool Server::start(const std::string& host,
                   const uint16_t port,
                   const int queue)
{
	static const int yes = 1; // Zmienna do ustawienia opcji `SO_REUSEADDR`
	if (m_sock) Wrapper::close(); // Zatrzymaj serwer, jeśli jest aktywny

	// Stwórz socket - IPv4, TCP
	int sock = ::socket(PF_INET, SOCK_STREAM, 0);

	if (sock == -1) return false;

	sockaddr_in sin;
	bool ok = true;

	// Uzupełnij strukturę adresu
	memset(&sin, 0, sizeof(sin));
	sin.sin_port = htons(port);
	sin.sin_family = AF_INET;

	// Konwertuj adres z łańcucha do liczby
	ok = ok && (::inet_pton(AF_INET, host.c_str(), &(sin.sin_addr)) == 1);

	// Ustaw opcję ponownego użycia adresu
	ok = ok && (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == 0);

	// Połącz gniazdo z adresem
	ok = ok && (::bind(sock, (sockaddr*) &sin, sizeof(sin)) == 0);

	// Rozpocznij nasłuchiwanie
	ok = ok && (::listen(sock, queue) == 0);

	// Zapisz deskryptor gniazda jeśli wszystko jest OK
	if (!ok) { ::close(sock); return false; }
	else m_sockets.push_back({ sock, POLLIN, 0 });

	return m_sock = sock;
}

bool Server::send(const int sock,
                  const std::vector<char>& data)
{
	if (!m_clients.contains(sock)) return false;
	else return m_clients[sock].send(data);
}

bool Server::send(const int sock, const std::string& data)
{
	if (!m_clients.contains(sock)) return false;
	else return m_clients[sock].send(data);
}

bool Server::recv(const int sock,
                  std::vector<char>& data,
                  size_t size)
{
	if (!m_clients.contains(sock)) return false;
	else return m_clients[sock].recv(data, size);
}

bool Server::recv(const int sock, std::string& data, size_t size)
{
	if (!m_clients.contains(sock)) return false;
	else return m_clients[sock].recv(data, size);
}

bool Server::close(int sock)
{
	if (!m_clients.contains(sock)) return false;

	auto st = m_sockets.begin() + 1;
	auto end = m_sockets.end();

	auto it = std::find_if(st, end,
	[sock] (const auto& i) -> bool
	{
		return i.fd == sock;
	});

	m_clients.erase(sock);
	m_sockets.erase(it);

	return true;
}

bool Server::loop(std::set<int>& read,
                  std::set<int>& write,
                  std::set<int>& open,
                  const time_t timeout)
{
	int count = poll(m_sockets.data(), m_sockets.size(), timeout);

	if (count < 0) return false;
	else if (count > 0)
	{
		if (m_sockets.front().revents & POLLIN)
		{
			if (accept()) open.insert(m_sockets.back().fd);
		}

		for (auto i = m_sockets.begin()+1; i != m_sockets.end();)
		{
			if (i->revents & (POLLHUP | POLLERR))
			{
				m_clients.erase(i->fd);
				i = m_sockets.erase(i);
			}
			else
			{
				if (i->revents & POLLIN) read.insert(i->fd);
				if (i->revents & POLLOUT) write.insert(i->fd);

				++i;
			}
		}
	}

	return true;
}

bool Server::flag(int sock, short flags, bool mode)
{
	auto st = m_sockets.begin() + 1;
	auto end = m_sockets.end();

	auto it = std::find_if(st, end,
	[sock] (const auto& i) -> bool
	{
		return i.fd == sock;
	});

	if (it == end) return false;

	if (mode) it->events |= flags;
	else it->events &= ~flags;

	return true;
}

std::string Server::name(int sock) const
{
	if (!m_clients.contains(sock)) return std::string();
	else return m_clients.at(sock).name();
}

bool Server::accept(void)
{
	if (!m_sock) return false;

	sockaddr_in sin; // Struktora pomocnicza na adres
	socklen_t size = sizeof(sin); // Długość adresu

	// Akceptuj nowe połączenie do serwera
	int sock = ::accept(m_sock, (sockaddr*) &sin, &size);
	SSL* ssl = nullptr;

	if (sock == -1) return false;
	else if (m_ctx)
	{
		ssl = SSL_new(m_ctx);

		if (SSL_set_fd(ssl, sock) != 1 ||
		    SSL_accept(ssl) != 1)
		{
			SSL_free(ssl);
			::close(sock);

			return false;
		}
	}

	m_sockets.push_back({ sock, POLLIN, 0 });
	m_clients.emplace(std::piecewise_construct,
	                  std::forward_as_tuple(sock),
	                  std::forward_as_tuple(m_ctx, ssl, sock));

	return true;
}

std::set<int> Server::list(void) const
{
	std::set<int> list;

	for (auto i = m_sockets.cbegin()+1; i != m_sockets.cend(); ++i)
	{
		list.insert(i->fd);
	}

	return list;
}
