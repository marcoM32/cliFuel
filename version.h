/**
 * cliFuel
 * Copyright (C) 2020-2023 Marco Magliano
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED

#define STR_IMPL_(x) #x
#define STR(x) STR_IMPL_(x)

#define CODE_NAME "Fog"
#define PROGRAM_NAME "cliFuel"
#define PROGRAM_VERSION_MAJOR 0
#define PROGRAM_VERSION_MINOR 0
#define PROGRAM_VERSION_PATCH 3
#define PROGRAM_VERSION "v" STR(PROGRAM_VERSION_MAJOR) "." STR(PROGRAM_VERSION_MINOR) \
			STR(PROGRAM_VERSION_PATCH) " (" CODE_NAME ")"

#endif // VERSION_H_INCLUDED