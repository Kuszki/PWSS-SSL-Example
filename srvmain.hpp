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

#ifndef SRVMAIN_HPP
#define SRVMAIN_HPP

#include <boost/program_options.hpp>
#include <boost/atomic.hpp>

#include <iostream>
#include <fstream>
#include <csignal>
#include <iomanip>
#include <string>
#include <chrono>

#include "server.hpp"

void signal(int value);

bool parser(int argc, char* argv[],
            std::string& host,
            uint16_t& port,
            std::string& cert,
            std::string& key,
            std::string& ca);

int main(int argc, char* argv[]);

std::string timestr(void);

#endif // SRVMAIN_HPP