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

boost::atomics::atomic_bool running = true;

void signal(int value)
{
	running = false;
}

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

		          ("host", value<>(&host)->default_value("127.0.0.1")->required(), "Listen address")
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
	}
	catch (const error &ex) { std::cerr << ex.what() << std::endl; return false; }

	return true;
}

int main(int argc, char* argv[])
{
	std::signal(SIGABRT, signal); // Błąd krytyczny (np. libc)
	std::signal(SIGINT, signal); // Kombinacja CTRL+C w terminalu
	std::signal(SIGTERM, signal); // Proces zakonczony (np. kill)

	std::map<int, std::stringstream> queue;
	std::set<int> read, write, open;

	std::string cert, key, ca, data;
	std::string host = "127.0.0.1";
	uint16_t port = 8080;

	Server* server = nullptr;

	if (!parser(argc, argv, host, port, cert, key, ca)) return 0;
	else server = new Server();

	if (!server->init(cert, key, ca))
	{
		std::cerr << "Unable to init SSL context" << std::endl; return -1;
	}

	if (!server->start(host, port))
	{
		std::cerr << "Unable to start server" << std::endl; return -1;
	}

	while (server->loop(read, write, open))
	{
		const std::string time = timestr() + '\t';

		std::string buff;
		buff.reserve(1024);

		for (const auto& i : open)
		{
			const std::string name = server->name(i);

			std::cout << "Accepted client: '" << name << "'" << std::endl;

			for (auto& [k, v] : queue)
			{
				v << time << name << " joined the chat" << std::endl;
			}

			queue.insert({ i, std::stringstream() });
		}

		for (const auto& i : read)
		{
			const std::string name = server->name(i);

			if (server->recv(i, buff)) for (auto& [k, v] : queue)
			{
				if (k != i) v << time << name << buff << std::endl;
			}
			else
			{
				std::cout << "Dissconnected client: '" << name << "'" << std::endl;

				queue.erase(i);

				for (auto& [k, v] : queue)
				{
					v << time << name << " leaved the chat" << std::endl;
				}
			}

			buff.clear();
		}

		for (const auto& i : write)
		{
			if (server->send(i, queue[i].str())) queue[i].clear();
		}

		open.clear();
		read.clear();
		write.clear();
	}

	delete server;
	return 0;
}

std::string timestr(void)
{
	auto now = std::chrono::system_clock::now();
	auto tm = std::chrono::system_clock::to_time_t(now);

	return (std::stringstream() << std::put_time(
	             std::localtime(&tm), "%Y-%m-%d %X")).str();
}
