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

#include <iostream>
#include <vector>
#include <string>

#include "server.hpp"

int main()
{
	std::string vect;

	Server srv;

	std::cout << "init " << srv.init("/home/kuszki/Projekty/SSL-Keys/server.crt",
	                                 "/home/kuszki/Projekty/SSL-Keys/server.key",
	                                 "/home/kuszki/Projekty/SSL-Keys/rootCA.crt")
	          << std::endl;

	std::cout << "start " << srv.start() << std::endl;
	std::cout << "accept " << srv.accept() << std::endl;

	auto id = *(srv.list().begin());

	std::cout << "name: " << srv.name(id) << std::endl;

	std::cout << "recv " << srv.recv(id, vect) << std::endl;
	std::cout << "send " << srv.send(id, "urtyurtyurty")  << std::endl;

	std::cout << vect << std::endl;

	return 0;
}
