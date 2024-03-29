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

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <boost/program_options.hpp>

#include <iostream>
#include <fstream>
#include <string>

#include "wrapper.hpp"

//! Przetwarzanie argumentów programu.
bool parser(int argc, /*!< [in] Liczba argumentów. */
		  char* argv[], /*!< [in] Lista argumentów. */
		  std::string& host, /*!< [out] Nazwa hosta. */
		  uint16_t& port, /*!< [out] Numer portu. */
		  std::string& cert, /*!< [out] Ścieżka pliku z certyfikatem. */
		  std::string& key, /*!< [out] Ścieżka pliku z kluczem prywatnym. */
		  std::string& ca /*!< [out] Ścieżka pliku z certyfikatem głównym. */);

//! Pobranie aktualnego czasu w formacie tekstowym.
std::string timestr(void);

#endif // OPTIONS_HPP
