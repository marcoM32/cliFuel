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
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <map.h>
#ifdef FILE_DOWNLOAD
#include <time.h>
#include <unistd.h>
#endif // FILE_DOWNLOAD
#ifdef ANIMATION
#ifdef _WIN32
#include <Windows.h>
#else
#ifndef _UNISTD_H
#include <unistd.h>
#endif // _UNISTD_H
#endif
#include <statusbar.h>
#include <pthread.h>
#endif // ANIMATION
#include "opendata.h"

#define PROGRAM_VERSION "v0.02"

static struct option long_options[] = {
    { "query", required_argument, NULL, 'q' },
    { "search-only", no_argument, NULL, 's' },
    { "ignore-cache", no_argument, NULL, 'u' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

typedef map_t(opendata_result_t*) opendata_map_t;

#ifdef ANIMATION
static void * progress(void *);
#endif // ANIMATION

static void helpme();

#endif // MAIN_H_INCLUDED
