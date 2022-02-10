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

#include "client.hpp"

Client::Client(void)
{
	m_method = TLS_client_method();
}

Client::Client(SSL_CTX* ctx, SSL* ssl, int sock)
     : Client()
{
	m_shared = true;

	m_ctx = ctx;
	m_ssl = ssl;
	m_sock = sock;
}

Client::~Client(void) {}

bool Client::open(const std::string& host, const uint16_t port)
{
	if (m_sock) this->close();

	const std::string ports = std::to_string(port);
	int sockfd(0);

	addrinfo hints, *servinfo, *p;

	// Uzupełnij strukturę podpowiedzi
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Pobierz informacje o hoście zgodnie z podpowiedziami
	const int ret = ::getaddrinfo(host.c_str(), ports.c_str(), &hints, &servinfo);

	if (ret != 0) return false;

	// Wyszukaj pierwszy pasujący adres
	for (p = servinfo; p != nullptr; p = p->ai_next)
	{
		// Na podstawie danych spróbuj utworzyć gniazdo
		if ((sockfd = ::socket(p->ai_family,
		                       p->ai_socktype,
		                       p->ai_protocol)) == -1)
		{
			sockfd = 0; // Gdy błąd - przejdź dalej
			continue; // do kolejnego elementu
		}

		// Spróbuj nawiązać połaczenie
		if (::connect(sockfd,
		              p->ai_addr,
		              p->ai_addrlen) == -1)
		{
			::close(sockfd); // Gdy błąd - zamknij
			sockfd = 0; // gniazdo po czym przejdź
			continue; // dalej do kolejnego elemenut
		}
		else break; // Gdy wszystko OK pomiń resztę elementów
	}

	// Zwolnij zasoby informacji o adresie
	freeaddrinfo(servinfo);

	if (sockfd == 0) return false;
	else m_sock = sockfd;

	if (!m_ctx) return true;

	SSL* ssl = SSL_new(m_ctx);

	if (!ssl) return false;

	if (SSL_set_fd(ssl, sockfd) == 1 &&
	    SSL_connect(ssl) == 1)
	{
		m_ssl = ssl;
	}
	else SSL_free(ssl);

	return m_ssl != nullptr;
}

bool Client::send(const std::vector<char>& data)
{
	if (!m_sock) return false;

	const char* prt = data.data();
	int size = data.size();

	if (m_ssl) size -= SSL_write(m_ssl, prt, size);
	else size -= ::send(m_sock, prt, size, 0);

	return size == 0;
}

bool Client::send(const std::string& data)
{
	if (!m_sock) return false;

	const char* prt = data.data();
	int size = data.size();

	if (m_ssl) size -= SSL_write(m_ssl, prt, size);
	else size -= ::send(m_sock, prt, size, 0);

	return size == 0;
}

bool Client::recv(std::vector<char>& data, size_t size)
{
	if (!m_sock) return false;

	char buff[size];
	int num = 0;

	if (m_ssl) num = SSL_read(m_ssl, buff, size);
	else num = ::recv(m_sock, buff, size, 0);

	if (num <= 0) return false;
	else std::copy(buff, buff + num,
	               std::back_inserter(data));

	return true;
}

bool Client::recv(std::string& data, size_t size)
{
	if (!m_sock) return false;

	char buff[size];
	int num = 0;

	if (m_ssl) num = SSL_read(m_ssl, buff, size);
	else num = ::recv(m_sock, buff, size, 0);

	if (num <= 0) return false;
	else data.append(buff, num);

	return true;
}
