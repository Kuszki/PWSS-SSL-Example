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

#include "options.hpp"

bool parser(int argc, char* argv[],
            std::string& host,
            uint16_t& port,
            std::string& cert,
            std::string& key,
            std::string& ca)
{
	using namespace boost::program_options;

	try
	{
		options_description desc("Options"); // Schemat opcji
		variables_map vm; // Kontener na pozyskane opcje

		desc.add_options() // Utworzenie listy opcji

		          ("host", value<>(&host)->default_value("127.0.0.1")->required(), "Host name")
		          ("port", value<>(&port)->default_value(8080)->required(), "Port number")
		          ("cert", value<>(&cert)->required(), "X509 certyficate file")
		          ("key", value<>(&key)->required(), "Private key file")
		          ("ca", value<>(&ca)->required(), "CA root certyficate file")

		          ("config", value<std::string>()->required(), "Use config form selected file")

		          ("version,v", "Display program version")
		          ("help,h", "Display help message");

		// Przetworzenie opcji programu i zapis w kontenerze
		store(parse_command_line(argc, argv, desc), vm);

		// Jeśli wybrano "--version lub -v" - wyświetlenie wersji i zakończenie funkcji
		if (vm.count("version")) { std::cout << Wrapper::version() << std::endl; return false; }

		// Jeśli wybrano "--help lub -h" - wyświetlenie pomocy i zakończenie funkcji
		else if (vm.count("help")) { std::cout << desc << std::endl; return false; }

		// Jeśli wybrano opcję wczytania ustawień s pliku - wczytanie ich z pliku
		else if (vm.count("config"))
		{
			std::ifstream ifs(vm["config"].as<std::string>());
			if (ifs) store(parse_config_file(ifs, desc), vm);
		}

		// Jeśli opcje nie zawierają plików certyfikatów i klucza - zwrócenie błędu
		if (!vm.count("cert") || !vm.count("key") || !vm.count("ca"))
		{
			std::cout << desc << std::endl; return 0;
		}

		vm.notify(); // Aktualizacja powiązanych zmiennych na podstawie pozyskanych wartości
	}

	// Obsługa wyjątku podczas przetwarzania opcji programu - wyświetlenie informacji
	catch (const error &ex) { std::cerr << ex.what() << std::endl; return false; }

	return true; // Informacja o poprawnym przetworzeniu opcji programu
}

std::string timestr(void)
{
	char buffer[100]; // Bufor na łańcuch
	time_t rawtime; // POSIXowy czas
	tm* timeinfo; // Struktura czasu

	rawtime = time(nullptr); // Pobranie czasu
	timeinfo = localtime(&rawtime); // Uzupełnienie struktury

	// Pobranie czasu w formacie tekstowym
	strftime(buffer, sizeof(buffer),
	         "%d.%m.%Y %H:%M:%S\t",
	         timeinfo);

	return buffer; // Zwrócenie łańcucha
}
