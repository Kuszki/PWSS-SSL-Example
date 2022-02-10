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

#include "wrapper.hpp"

Wrapper::Wrapper(void) {}

Wrapper::Wrapper(Wrapper&& wrapper)
{
	m_ctx = wrapper.m_ctx;
	m_ssl = wrapper.m_ssl;

	m_shared = wrapper.m_shared;
	m_sock = wrapper.m_sock;

	wrapper.m_ctx = nullptr;
	wrapper.m_ssl = nullptr;

	wrapper.m_shared = false;
	wrapper.m_sock = 0;
}

Wrapper::~Wrapper(void)
{
	if (m_sock || m_ssl) this->close();

	if (m_ctx && !m_shared) SSL_CTX_free(m_ctx);
}

bool Wrapper::init(const std::string& cert, const std::string& key, const std::string& ca)
{
	if (m_sock || m_ssl) return false;

	if (m_ctx && !m_shared) SSL_CTX_free(m_ctx);

	m_ctx = SSL_CTX_new(m_method);
	m_shared = false;

	bool ok = m_ctx != nullptr;

	if (!cert.empty())
		ok = ok && SSL_CTX_use_certificate_file(m_ctx, cert.c_str(), SSL_FILETYPE_PEM);

	if (!key.empty())
		ok = ok && SSL_CTX_use_PrivateKey_file(m_ctx, key.c_str(), SSL_FILETYPE_PEM);

	if (!ca.empty())
		ok = ok && SSL_CTX_load_verify_locations(m_ctx, ca.c_str(), nullptr);

	if (ok)
	{
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER, nullptr);
		SSL_CTX_set_options(m_ctx, SSL_OP_NO_SSLv2);
	}

	return ok;
}

bool Wrapper::init(SSL_CTX* ctx)
{
	m_shared = true;
	m_ctx = ctx;

	return true;
}

bool Wrapper::close(void)
{
	if (!m_sock) return false;

	if (m_ssl)
	{
		SSL_shutdown(m_ssl);
		SSL_free(m_ssl);
	}

	::close(m_sock);

	m_ssl = nullptr;
	m_sock = 0;

	return true;
}

int Wrapper::sock(void) const
{
	return m_sock;
}

std::string Wrapper::name(void) const
{
	if (!m_sock) return std::string();

	if (!m_ssl)
	{
		sockaddr_in addr;
		socklen_t len = sizeof(addr);

		// Pobierz dane związane z gniazdem
		getpeername(m_sock, (sockaddr*) &addr, &len);

		// Konwertuj adres z liczby na łańcuch
		return inet_ntoa(addr.sin_addr);
	}

	X509 *cert = SSL_get_peer_certificate(m_ssl);

	if (cert != nullptr)
	{
		auto name = X509_get_subject_name(cert);
		auto entry = X509_NAME_get_entry(name, 5);
		auto data = X509_NAME_ENTRY_get_data(entry);

		std::string peer = (char*) ASN1_STRING_get0_data(data);

		X509_free(cert);

		return std::move(peer);
	}
	else return std::string();
}
