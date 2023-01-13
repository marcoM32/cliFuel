/**
 * cliFuel
 * Copyright (C) 2020-2022 Marco Magliano
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
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <map.h>
#include <dmt.h>
#ifdef FILE_DOWNLOAD
#include <time.h>
#endif // FILE_DOWNLOAD
#include <unistd.h>
#ifdef ANIMATION
#ifdef _WIN32
#include <winsock2.h>
#include <Windows.h>
#endif // _WIN32
#include <statusbar.h>
#include <pthread.h>
#endif // ANIMATION
#include <libgen.h>
#include "opendata.h"
#include "version.h"
#ifdef HTML_OUTPUT
#include <stdarg.h>
#endif // HTML_OUTPUT

#ifdef HTML_OUTPUT
#define fprintf(f_, ...) html_fprintf(f_, HTML_TAG_OPEN_PARAGRAPH, HTML_TAG_CLOSE_PARAGRAPH, ##__VA_ARGS__)
#define HTML_TAG_OPEN_PAGE "<!DOCTYPE html><html><body>"
#define HTML_TAG_CLOSE_PAGE "</body></html>"
#define HTML_TAG_OPEN_PARAGRAPH "<p>"
#define HTML_TAG_CLOSE_PARAGRAPH "</p>"
#define NEW_LINE "<br/>"
#define TAB "&emsp;"
#else
#define NEW_LINE "\n"
#define TAB "\t"
#endif

static struct option long_options[] = {
    { "query", required_argument, NULL, 'q' },
    { "type", required_argument, NULL, 't' },
    { "path", required_argument, NULL, 'p' },
    { "search-only", no_argument, NULL, 's' },
    { "ignore-cache", no_argument, NULL, 'u' },
    { "ignore-old", no_argument, NULL, 'o' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

typedef map_t(opendata_result_t*) opendata_map_t;

#ifdef ANIMATION
static void * progress(void*);
#endif // ANIMATION

static char* get_file_path(const char*, const char*);

static int is_directory(const char*);

static void create_directory(const char*);

static void helpme();

#ifdef HTML_OUTPUT
static void html_fprintf(FILE *, const char*, const char*, const char*, ...);

static void on_exit_close_tag();
#endif // HTML_OUTPUT

#endif // MAIN_H_INCLUDED
