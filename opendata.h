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

#ifndef OPENDATA_H_INCLUDED
#define OPENDATA_H_INCLUDED

#ifdef COLOR
#define _GNU_SOURCE 1
#endif // COLOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <log.h>
#include <dmt.h>
#include <curl/curl.h>
#include <csvparser.h>

#ifdef COLOR
#include <time.h>
#endif // COLOR

#define CACHE_DIR "cache"

#define ANAGRAFIA_IMPIANTI_FILE "anagrafica_impianti_%y_%m_%d.csv"
#define PREZZI_FILE "prezzi_%y_%m_%d.csv"

#define FILENAME(dir, filename) dir "/" filename

#define ANAGRAFIA_IMPIANTI_URL "https://www.mise.gov.it/images/exportCSV/anagrafica_impianti_attivi.csv"
#define PREZZI_URL "https://www.mise.gov.it/images/exportCSV/prezzo_alle_8.csv"

#define FILE_SEPARATOR ";"
#define FILE_HEADER false

#ifdef COLOR
#define COLOR_START_PATTERN(c) "\e[1;" #c "m"
#define COLOR_END_PATTERN "\e[0m"
#endif // COLOR

#define QUERY_PREFIX_ID "id:"

enum item_age {NEW = 0, OK, OLD};

enum service {SELF = 0, NOT_SELF};

typedef struct station {
    int id;
    char* name;
    char* type;
    char* address;
    char* town;
    struct station* next;
} station_t;

typedef struct price {
    int id;
    char* fuelDesc;
    float price;
    enum service self;
    char* lastUpdate;
    enum item_age is_old;
    struct price* next;
} price_t;

typedef struct {
    struct station *station;
    struct price *price;
} opendata_result_t;

bool download(const char*, const char*);

station_t* stationFinder(char*, char*, bool, char*);

price_t* priceFinder(char*, char*, bool, station_t*, char*);

void freeStationList(station_t*);

void freePriceList(price_t*);

enum item_age is_old_data(const char*);

#ifdef COLOR
char* make_alert(const price_t*);
#endif // COLOR

#endif // OPENDATA_H_INCLUDED
