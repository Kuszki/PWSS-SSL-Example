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
	m_method = TLS_client_method(); // Wybór metody SSL
}

Client::Client(SSL_CTX* ctx, SSL* ssl, int sock)
     : Client() // Wywołaj konstruktor domyślny
{
	m_shared = true; // Zapamiętaj status kontekstu

	m_ctx = ctx; // Zapamiętaj kontekst
	m_ssl = ssl; // Zapamiętaj wrapper SSL
	m_sock = sock; // Zapamiętaj deskryptor gniazda
}

Client::~Client(void) {} // Destruktor dla zachowania porządku

Wrapper::error Client::open(const std::string& host, const uint16_t port)
{
	if (m_sock) this->close(); // Zamknij poprzednie połaczenie, jeśli aktywne

	const std::string ports = std::to_string(port); // Konwertuj port na łańcuch
	int sockfd(0); // Roboczy deskryptor gniazda

	addrinfo hints, *servinfo, *p; // Wskaźniki robocze na wyniki wyszukiwań

	// Uzupełnij strukturę podpowiedzi
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Pobierz informacje o hoście zgodnie z podpowiedziami
	const int ret = ::getaddrinfo(host.c_str(), ports.c_str(), &hints, &servinfo);

	if (ret != 0) return error::addrinfo_call_error; // Jeśli nie udało się uzyskać odpowiedzi - zakończ

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

	freeaddrinfo(servinfo); // Zwolnij zasoby informacji o adresie

	if (sockfd == 0) return error::no_server_found; // Jeśli nie udało się połączyć - zakończ
	else m_sock = sockfd; // W przeciwnym razie zapamiętaj deskryptor gniazda

	// W przypadku, gdy nie ma kontekstu SSL, połaczenie nie będzie szyfrowane
	if (!m_ctx) return error::no_error; // Jeśli kontekst SSL nie istnieje - zakończ

	SSL* ssl = SSL_new(m_ctx); // Utwórz obiekt SSL na podstawie kontekstu

	if (!ssl) return error::create_sslobj_fail; // Jeśli nie udało się utworzyć obiektu - zakończ

	if (SSL_set_fd(ssl, sockfd) == 1 && // Powiąż gniazdo z obiektem SSL
	    SSL_connect(ssl) == 1) // Wynegocjuj warunki połączenia SSL
	{
		m_ssl = ssl; // Jeśli wszystko się powiodło - zapamiętaj obiekt SSL
	}
	else SSL_free(ssl); // Jeśli nie - zwolnij obiekt SSL

	return m_ssl != nullptr ? error::no_error : error::sslwrap_call_error; // Zwróć powodzenie operacji
}

Wrapper::error Client::send(const std::vector<char>& data)
{
	if (!m_sock) return error::no_active_socket; // Jeśli połczenie nie jest aktywne - zwróć błąd

	const char* prt = data.data(); // Dane do wysłania
	int size = data.size(); // Liczba bajtów do wysłania

	// Jeśli połączenie jest zaszyfrowane - wyślij stosując szyfrowanie
	if (m_ssl) size -= SSL_write(m_ssl, prt, size);

	// W przypadku nieszyfrowanego połaczenia - użyj klasycznego send
	else size -= ::send(m_sock, prt, size, 0);

	return size == 0 ? error::no_error : error::send_call_error; // Jeśli wysłano wszystkie dane - zwróć powodzenie
}

Wrapper::error Client::send(const std::string& data)
{
	if (!m_sock) return error::no_active_socket; // Jeśli połczenie nie jest aktywne - zwróć błąd

	const char* prt = data.data(); // Dane do wysłania
	int size = data.size(); // Liczba bajtów do wysłania

	// Jeśli połączenie jest zaszyfrowane - wyślij stosując szyfrowanie
	if (m_ssl) size -= SSL_write(m_ssl, prt, size);

	// W przypadku nieszyfrowanego połaczenia - użyj klasycznego send
	else size -= ::send(m_sock, prt, size, 0);

	return size == 0 ? error::no_error : error::send_call_error; // Jeśli wysłano wszystkie dane - zwróć powodzenie
}

Wrapper::error Client::recv(std::vector<char>& data, size_t size)
{
	if (!m_sock) return error::no_active_socket; // Jeśli połczenie nie jest aktywne - zwróć błąd

	char buff[size]; // Bufor na dane
	int num = 0; // Liczba odebranych bajtów

	// Jeśli połączenie jest zaszyfrowane - czytaj stosując szyfrowanie
	if (m_ssl) num = SSL_read(m_ssl, buff, size);

	// W przypadku nieszyfrowanego połaczenia - użyj klasycznego recv
	else num = ::recv(m_sock, buff, size, 0);

	if (num <= 0) return error::recv_call_error; // W przypadku błędu - zakończ

	// W przypadku powodzenia - skopiuj odebrane dane z bufora do kontenera
	else std::copy(buff, buff + num,
	               std::back_inserter(data));

	return error::no_error; // Zwróć powodzenie operacji
}

Wrapper::error Client::recv(std::string& data, size_t size)
{
	if (!m_sock) return error::no_active_socket; // Jeśli połczenie nie jest aktywne - zwróć błąd

	char buff[size]; // Bufor na dane
	int num = 0; // Liczba odebranych bajtów

	// Jeśli połączenie jest zaszyfrowane - czytaj stosując szyfrowanie
	if (m_ssl) num = SSL_read(m_ssl, buff, size);

	// W przypadku nieszyfrowanego połaczenia - użyj klasycznego recv
	else num = ::recv(m_sock, buff, size, 0);

	if (num <= 0) return error::recv_call_error; // W przypadku błędu - zakończ
	else data.append(buff, num); // Dopisz dane do bufora

	return error::no_error; // Zwróć powodzenie operacji
}
