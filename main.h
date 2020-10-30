/**
 * cliFuel
 * Copyright (C) 2020 Marco Magliano
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

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>
#include <getopt.h>
#include <map.h>
#ifdef FILE_DOWNLOAD
    #include <time.h>
    #include <unistd.h>
#endif // FILE_DOWNLOAD
#include "opendata.h"

#define PROGRAM_VERSION "v0.01"

static struct option long_options[] = {
    { "query", required_argument, NULL, 'q' },
    { "search-only", no_argument, NULL, 's' },
    { "ignore-cache", no_argument, NULL, 'u' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' }
};

typedef map_t(opendata_result_t*) opendata_map_t;

#endif // MAIN_H_INCLUDED
