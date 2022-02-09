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

#include "client.hpp"

int main(int argc, char* argp[])
{
	std::string vect;



	Client cli;

	std::cout << "init " << cli.init("/home/kuszki/Projekty/SSL-Keys/client_a.crt",
	                                 "/home/kuszki/Projekty/SSL-Keys/client_a.key",
	                                 "/home/kuszki/Projekty/SSL-Keys/rootCA.crt")
	          << std::endl;

	std::cout << "open " << cli.open("localhost", 8080) << std::endl;
	std::cout << "send " << cli.send("asdasdasd") << std::endl;
	std::cout << "recv " << cli.recv(vect) << std::endl;

	std::cout << vect << std::endl;

	return 0;
}
