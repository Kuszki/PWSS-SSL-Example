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

#include "wrapper.hpp"

Wrapper::Wrapper(void) {}

Wrapper::Wrapper(Wrapper&& wrapper)
{
	m_ctx = wrapper.m_ctx; // Przenieś kontekst
	m_ssl = wrapper.m_ssl; // Przenieś obiekt SSL

	m_shared = wrapper.m_shared; // Skopiuj informację o współdzieleniu kontekstu
	m_sock = wrapper.m_sock; // Skopiuj deskryptor gniazda

	wrapper.m_ctx = nullptr; // Wyzeruj kontekst SSL
	wrapper.m_ssl = nullptr; // Wyzeruj obiekt SSL

	wrapper.m_shared = false; // Wyzeruj informację o współdzieleniu
	wrapper.m_sock = 0; // Wyzeruj deskryptor gniazda
}

Wrapper::~Wrapper(void)
{
	if (m_sock || m_ssl) this->close(); // Jeśli obiekt jest aktywny - zamknij go

	if (m_ctx && !m_shared) SSL_CTX_free(m_ctx); // Jeśli kontekst jest unikatowy - zwolnij go
}

Wrapper::error Wrapper::init(const std::string& cert, const std::string& key, const std::string& ca)
{
	if (m_sock || m_ssl) return error::is_active_socket; // Jeśli obiekt jest aktywny - zakończ

	if (m_ctx && !m_shared) SSL_CTX_free(m_ctx); // Jeśli kontekst jest unikatowy - zwolnij go

	m_ctx = SSL_CTX_new(m_method); // Utwórz nowy kontekst SSL
	m_shared = false; // Zeruj flagę współdzielenia kontekstu

	bool ok = m_ctx != nullptr; // Flaga powodzenia operacji

	if (!cert.empty()) // Wczytaj certyfikat, jeśli podano
		ok = ok && SSL_CTX_use_certificate_file(m_ctx, cert.c_str(), SSL_FILETYPE_PEM);

	if (!key.empty()) // Wczytaj klucz prywatny, jeśli podano
		ok = ok && SSL_CTX_use_PrivateKey_file(m_ctx, key.c_str(), SSL_FILETYPE_PEM);

	if (!ca.empty()) // Wczytaj certyfikat główny, jeśli podano
		ok = ok && SSL_CTX_load_verify_locations(m_ctx, ca.c_str(), nullptr);

	if (ok) // Jeśli wszystko OK, ustaw dodatkowe flagi
	{
		// Weryfikuj certyfikat peera oraz przerywaj handshake gdy brak/błędny certyfikat
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
		SSL_CTX_set_options(m_ctx, SSL_OP_NO_SSLv2); // Wyłącz starą wersję SSL
	}

	return ok ? error::no_error : error::create_context_fail; // Zwróć powodzenie operacji
}

Wrapper::error Wrapper::init(SSL_CTX* ctx)
{
	// Jeśli kontekst jest unikatowy - zwolnij go
	if (m_ctx && !m_shared) SSL_CTX_free(m_ctx);

	m_shared = true; // Ustal flagę współdzielenia
	m_ctx = ctx; // Przypisz współdzielony kontekst

	return error::no_error; // Zwróć powodzenie operacji
}

Wrapper::error Wrapper::close(void)
{
	if (!m_sock) return error::is_active_socket; // Jeśli obiekt nieaktywny - zakończ

	if (m_ssl) // Jeśli połączenie jest zaszyfrowane
	{
		SSL_shutdown(m_ssl); // Zamknij połączenie SSL
		SSL_free(m_ssl); // Zwolnij zasoby obiektu SSL
	}

	::close(m_sock); // Zamknij gniazdo połączenia

	m_ssl = nullptr; // Zeruj wskaźnik na obiekt SSL
	m_sock = 0; // Zeruj deskryptor gniazda

	return error::no_error; // Zwróć powodzenie operacji
}

int Wrapper::sock(void) const
{
	return m_sock; // Zwróć deskryptor gniazda
}

std::string Wrapper::name(void) const
{
	if (!m_sock) return std::string(); // Jeśli obiekt nieaktywny - zakończ

	sockaddr_in addr; // Struktura adresu
	socklen_t len = sizeof(addr); // Długość struktury
	std::string peraddr;

	// Pobierz dane związane z gniazdem i uzupełnij ciąg
	if (getpeername(m_sock, (sockaddr*) &addr, &len) != 0)
		peraddr = inet_ntoa(addr.sin_addr);

	// Gdy nie udało się pozyskać danych ustal nazwę na "nieznany"
	if (peraddr.empty()) peraddr = std::string("unknown peer");

	// Jeśli połączenie nie jest zaszyfrowane zwróć pobrany adres
	if (!m_ssl) return peraddr;

	// Pobierz certyfikat od połączonego partnera
	X509 *cert = SSL_get_peer_certificate(m_ssl);

	if (cert != nullptr) // Pobierz informacje o partnerze z certyfikatu
	{
		// Przygotuj smart-pointer ułatwiający sprzątanie
		// w przypadku błędu podczas pozyskiwania nazwy
		std::unique_ptr<X509, void(*)(X509*)> ptr(cert,
		[] (X509* p)
		{
			if (p) X509_free(p);
		});

		auto name = X509_get_subject_name(cert); // Dane partnera
		if (name == nullptr) return peraddr; // Zwróć adres gdy błąd

		// Wyszukaj pierwsze pole odpowiadające nazwie (CN)
		const int id = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
		if (id == -1) return peraddr; // Zwróć adres gdy błąd

		auto entry = X509_NAME_get_entry(name, id); // Obiekt nazwy (CN)
		if (entry == nullptr) return peraddr; // Zwróć adres gdy błąd

		auto data = X509_NAME_ENTRY_get_data(entry); // Dane obiektu
		if (data == nullptr) return peraddr; // Zwróć adres gdy błąd

		return (char*) ASN1_STRING_get0_data(data); // Zwróć nazwę partnera
	}
	else return peraddr; // Zwróć domyślną nazwę gdy błąd
}

std::string Wrapper::cipher(void) const
{
	if (!m_ssl) return "noone";
	else return SSL_get_cipher(m_ssl);
}

bool Wrapper::is_ok(Wrapper::error err)
{
	return err == error::no_error;
}

std::string Wrapper::version(bool sh)
{
	if (sh) return GIT_HASH_SHORT;
	else return GIT_HASH_LONG;
}

std::string Wrapper::compiler(void)
{
	return COMPILER_NAME " v" COMPILER_VERSION " @ " SYSTEM_NAME;
}
