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

#include "climain.hpp"

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
		options_description desc("Options");
		variables_map vm;

		desc.add_options()

		          ("host", value<>(&host)->default_value("localhost")->required(), "Host name")
		          ("port", value<>(&port)->default_value(8080)->required(), "Port number")
		          ("cert", value<>(&cert)->required(), "X509 certyficate file")
		          ("key", value<>(&key)->required(), "Private key file")
		          ("ca", value<>(&ca)->required(), "CA root certyficate file")

		          ("config", value<std::string>()->required(), "Use config form selected file")

		          ("help,h", "Display help message");

		store(parse_command_line(argc, argv, desc), vm);

		if (vm.count("help")) { std::cout << desc << std::endl; return false; }
		else if (vm.count("config"))
		{
			std::ifstream ifs(vm["config"].as<std::string>().c_str());
			if (ifs) store(parse_config_file(ifs, desc), vm);
		}

		if (!vm.count("cert") || !vm.count("key") || !vm.count("ca"))
		{
			std::cout << desc << std::endl; return 0;
		}

		vm.notify();
	}
	catch (const error &ex) { std::cerr << ex.what() << std::endl; return false; }

	return true;
}

int main(int argc, char* argv[])
{
	std::string cert, key, ca;
	std::string host = "localhost";
	uint16_t port = 8080;

	std::string buff;
	buff.reserve(1024);

	Client* client = nullptr;

	if (!parser(argc, argv, host, port, cert, key, ca)) return 0;
	else client = new Client();

	if (!client->init(cert, key, ca))
	{
		std::cerr << "Unable to init SSL context" << std::endl; return -1;
	}

	if (!client->open(host, port))
	{
		std::cerr << "Unable to open connection" << std::endl; return -1;
	}

	pollfd list[] = {
	     { client->sock(), POLLIN, 0},
	     { 0, POLLIN, 0 }
	};

	std::cin.clear();

	while (poll(list, 2, -1) > 0)
	{
		if (list[0].revents & POLLIN)
		{
			if (!client->recv(buff)) return -1;
			else { std::cout << buff; buff.clear(); }
		}

		if (list[1].revents & POLLIN)
		{
			std::getline(std::cin, buff);

			if (!client->send(buff)) return -2;
			else buff.clear();
		}
	}

	delete client;

	return 0;
}
