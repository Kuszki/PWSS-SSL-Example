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

#include "srvmain.hpp"

int main(int argc, char* argv[])
{
	std::map<int, std::string> queue; // Kolejka komunikatów do klientów
	std::set<int> read, write, open; // Zbiory na gotowych klientów

	std::string cert, key, ca; // Opcje kontekstu SSL
	std::string host = "127.0.0.1"; // Adres nasłuchiwania
	uint16_t port = 8080; // Numer portu serwera

	std::string buff; // Bufor na dane
	buff.reserve(1024); // Rezerwacja pamięci

	Server* server = nullptr; // Instancja serwera

	// Przetworzenie opcji programu - w przypadku błędu zakończ program
	if (!parser(argc, argv, host, port, cert, key, ca)) return 0;
	else server = new Server(); // W przeciwnym razie utworzenie instancji serwera

	if (!server->init(cert, key, ca)) // Inicjuj kontekst SSL
	{
		std::cerr << "Unable to init SSL context" << std::endl; return -1;
	}

	if (!server->start(host, port)) // Uruchom serwer
	{
		std::cerr << "Unable to start server" << std::endl; return -1;
	}

	// Pętla będzie wykonywana az do błędu `poll` lub odebrania sygnału zakończenia
	while (server->loop(read, write, open)) // Sprawdź gotowość klientów
	{
		const auto list = server->list(); // Pobierz listę klientów
		const auto time = timestr(); // Pobierz aktualny czas

		for (const auto& i : open) // Sprawdź nowe połączenia
		{
			// Pobierz nazwę klienta na podstawie certyfikatu
			const std::string name = server->name(i);

			// Dodaj do kolejki wszystkich klientów informację o nowym połączeniu
			for (const auto& k : list) if (k != i)
			{
				queue[k] += time + name + " joined the chat\n"; // Wiadomość
				server->flag(k, POLLOUT, true); // Monitoruj gniazdo (zapis)
			}

			std::cout << time << "Accepted client: '" << name << "'" << std::endl;
		}

		for (const auto& i : read) // Pobierz dane od klientów
		{
			// Pobierz nazwę klienta na podstawie certyfikatu
			const std::string name = server->name(i);

			// Jeśli pobrano nowe dane, dodaj komunikat do kolekji klientów
			if (server->recv(i, buff)) for (const auto& k : list)
			{
				if (k != i) // Pomiń bieżącego klienta
				{
					queue[k] += time + name + " > " + buff + '\n'; // Wiadomość
					server->flag(k, POLLOUT, true); // Monitoruj gniazdo (zapis)
				}
			}
			else // Jeśli nie udało się pobrać danych (rozłączenie lub błąd)
			{
				// Dodaj komunikat o rozłączeniu dla pozostałych klientów
				for (const auto& k : list) if (k != i)
				{
					queue[k] += time + name + " leaved the chat\n"; // Wiadomość
					server->flag(k, POLLOUT, true); // Monitoruj gniazdo (zapis)
				}

				server->close(i); // Zamknij połączenie
				queue.erase(i); // Usuń kolejkę klienta

				std::cout << time << "Dissconnected client: '" << name << "'" << std::endl;
			}

			buff.clear(); // Wyczyść roboczy bufor na dane
		}

		for (const auto& i : write) if (queue.contains(i)) // Wyślij dane do klientów
		{
			if (server->send(i, queue[i])) // Jeśli udało się wysłać dane
			{
				server->flag(i, POLLOUT, false); // Nie monitoruj możliwości zapisu
				queue.erase(i); // Usuń kolejkę zapisu klienta (dane są wysłane)
			}
		}

		open.clear(); // Wyczyść listę nowych klientów
		read.clear(); // Wyczyść listę gotowych do odczytu
		write.clear(); // Wyczyść listę gotowych do zapisu
	}

	delete server; // Usuń instancję serwera
	return 0; // Zakończ program
}
