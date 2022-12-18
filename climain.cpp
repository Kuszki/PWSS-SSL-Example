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

#include "climain.hpp"

int main(int argc, char* argv[])
{
	std::string cert, key, ca; // Konfiguracja SSL
	std::string host = "localhost"; // Nazwa hosta
	uint16_t port = 8080; // Numer portu serwera

	std::string buff; // Bufor na dane
	buff.reserve(1024); // Rezerwacja pamięci

	std::unique_ptr<Client> client = nullptr; // Instancja klienta

	// Przetworzenie opcji programu - w przypadku błędu zakończ program
	if (!parser(argc, argv, host, port, cert, key, ca)) return 0;
	else client = std::make_unique<Client>(); // W przeciwnym wypadku utwórz instancje klienta

	if (!Client::is_ok(client->init(cert, key, ca))) // Inicjuj kontekst SSL
	{
		std::cerr << "Unable to init SSL context" << std::endl; return -1;
	}

	if (!Client::is_ok(client->open(host, port))) // Nawiąż połączenie z serwerem
	{
		std::cerr << "Unable to open connection" << std::endl; return -1;
	}
	else
	{
		std::cout << "Connected with '" << client->name() << "'" << std::endl;
	}

	// Lista monitorowanych strumieni
	pollfd list[] =
	{
		{ client->sock(), POLLIN, 0}, // Serwer
		{ 0, POLLIN, 0 } // Standardowe wejście
	};

	std::cin.clear(); // Wyczyść standardowe wejście

	while (poll(list, 2, -1) > 0) // Sprawdź możliwości strumieni
	{
		if (list[0].revents & POLLIN) // Sprawdź, czy można odczytać z serwera
		{
			if (!Client::is_ok(client->recv(buff))) return -1; // W przypadku niepowodzenia zakończ program
			else { std::cout << buff; buff.clear(); } // Wyświetl dane i wyczyść bufor
		}

		if (list[1].revents & POLLIN) // Sprawdź, czy można odczytać z terminala
		{
			std::getline(std::cin, buff); // Pobierz dane z terminala

			if (!Client::is_ok(client->send(buff))) return -2; // Zakończ, jeśli nie udało się wysłać
			else buff.clear(); // W przeciwnym razie wyczyść bufor (do ponownego użycia)
		}
	}

	return 0; // Zakończ program
}
