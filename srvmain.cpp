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
		          ("ca", value<std::string>(&ca)->required(), "CA root certyficate file")

		          ("config", value<std::string>()->required(), "Use config form selected file")

		          ("help,h", "Display help message");

		store(parse_command_line(argc, argv, desc), vm);

		if (vm.count("help")) { std::cout << desc << std::endl; return false; }
		else if (vm.count("config"))
		{
			std::ifstream ifs(vm["config"].as<std::string>().c_str());
			if (ifs)
				store(parse_config_file(ifs, desc), vm);
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
	std::map<int, std::string> queue;
	std::set<int> read, write, open;

	std::string cert, key, ca, data;
	std::string host = "127.0.0.1";
	uint16_t port = 8080;

	std::string buff;
	buff.reserve(1024);

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
		if (queue.empty() && read.empty() && open.empty()) continue;

		const auto list = server->list();
		const auto time = timestr();

		for (const auto& i : open)
		{
			const std::string name = server->name(i);

			for (const auto& k : list) if (k != i)
			{
				queue[k] += time + name + " joined the chat\n";
				server->flag(k, POLLOUT, true);
			}

			std::cout << time << "Accepted client: '" << name << "'" << std::endl;
		}

		for (const auto& i : read)
		{
			const std::string name = server->name(i);

			if (server->recv(i, buff)) for (const auto& k : list)
			{
				if (k != i)
				{
					queue[k] += time + name + " > " + buff + '\n';
					server->flag(k, POLLOUT, true);
				}
			}
			else
			{
				for (const auto& k : list) if (k != i)
				{
					queue[k] += time + name + " leaved the chat\n";
					server->flag(k, POLLOUT, true);
				}

				server->close(i);
				queue.erase(i);

				std::cout << time << "Dissconnected client: '" << name << "'" << std::endl;
			}

			buff.clear();
		}

		for (const auto& i : write) if (queue.contains(i))
		{
			if (server->send(i, queue[i]))
			{
				server->flag(i, POLLOUT, false);
				queue.erase(i);
			}
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
	char buffer[100];
	time_t rawtime;
	tm* timeinfo;

	rawtime = time(nullptr);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer),
	         "%d.%m.%Y %H:%M:%S\t",
	         timeinfo);

	return buffer;
}
