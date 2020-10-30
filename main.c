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

#include "main.h"

int main(int argn, char* argv[]) {

    log_set_level(LOG_ERROR);

    char* query = "";
    bool searchonly = false;
    bool ignorecache = false;

    char option = '\0';
    while((option = getopt_long(argn, argv, "q:suvh", long_options, NULL)) != -1) {
        switch(option) {
        case 'q':
            if (optarg)
                query = optarg;
            break;
        case 's':
            searchonly = true;
            log_debug("Modalita' di sola ricerca impianti");
            break;
        case 'u':
            ignorecache = true;
            log_debug("Modalita' ignore cache attiva");
#ifdef NO_CACHE
            log_debug("Build type NO_CACHE");
#endif // NO_CACHE
            break;
        case 'v':
            log_set_level(LOG_DEBUG);
            break;
        case 'h':
            fprintf(stdout, "%s %s\n"
                    "\n"
                    "Copyright (C) 2020 Marco Magliano Free Software Foundation, Inc.\n"
                    "This is free software; see the source for copying conditions.\n"
                    "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n"
                    "PARTICULAR PURPOSE.\n"
                    "\n"
                    "Buildtime: %s - %s\n", "cliFuel", PROGRAM_VERSION, __DATE__, __TIME__);
            exit(EXIT_SUCCESS);
            break;
        case '?':
            fprintf(stdout, "Comando sconosciuto: %c\n", optopt);
            exit(EXIT_FAILURE);
            break;
        }
    }

    char* anagrafiaFilename = malloc((sizeof(char) * strlen(ANAGRAFIA_IMPIANTI_FILE)) + 1);
    char* priceFilename = malloc((sizeof(char) * strlen(PREZZI_FILE)) + 1);

    if(!anagrafiaFilename || !priceFilename) exit(EXIT_FAILURE);

    struct tm *timenow;
    time_t now = time(NULL);
    timenow = gmtime(&now);
    strftime(anagrafiaFilename, strlen(ANAGRAFIA_IMPIANTI_FILE) + 1, ANAGRAFIA_IMPIANTI_FILE, timenow);
    strftime(priceFilename, strlen(PREZZI_FILE) + 1, PREZZI_FILE, timenow);

#ifdef FILE_DOWNLOAD
    if(ignorecache || !(access(anagrafiaFilename, F_OK ) != -1))  {
        if(!download(ANAGRAFIA_IMPIANTI_URL, anagrafiaFilename)) {
            log_error("Impossibile scaricare il file %s", ANAGRAFIA_IMPIANTI_URL);
        } else {
            log_debug("File %s scaricato correttamente", ANAGRAFIA_IMPIANTI_URL);
        }
    } else {
        log_debug("File %s gia' in cache", ANAGRAFIA_IMPIANTI_URL);
    }

    if(ignorecache || !(access(priceFilename, F_OK ) != -1)) {
        if(!download(PREZZI_URL, priceFilename)) {
            log_error("Impossibile scaricare il file %s", PREZZI_URL);
        } else {
            log_debug("File %s scaricato correttamente", PREZZI_URL);
        }
    } else {
        log_debug("File %s gia' in cache", PREZZI_URL);
    }
#endif // FILE_DOWNLOAD

    station_t* stations = stationFinder(anagrafiaFilename, FILE_SEPARATOR, FILE_HEADER, query);
#ifdef DEBUG
    if(stations != NULL) {
        station_t *tmp = stations;
        while (tmp->next != NULL) {
            log_debug(u8"%d - %s - %s - %s\n", tmp->id, tmp->name, tmp->address, tmp->town);
            tmp = tmp->next;
        }
    }
#endif // DEBUG

    price_t* prices = priceFinder(priceFilename, FILE_SEPARATOR, FILE_HEADER, stations);
#ifdef DEBUG
    if(prices != NULL) {
        price_t *tmp = prices;
        while (tmp->next != NULL) {
            log_debug(u8"%s, %.3f euro %s\n", tmp->fuelDesc, tmp->price, tmp->self == 0 ? "non servito" : "servito");
            tmp = tmp->next;
        }
    }
#endif // DEBUG

    opendata_map_t map;
    map_init(&map);

    if(stations != NULL) {
        station_t *station = stations;
        while (station->next != NULL) {
            if(searchonly)
                printf( //
                       u8"* %s - %s - %s\n", //
                       station->name, //
                       station->address, //
                       station->town //
                );
            if(prices != NULL) {
                price_t *price = prices;
                while (price->next != NULL) {
                    if(price->id == station->id) {
                        if(!searchonly) {
                            opendata_result_t **val = map_get(&map, price->fuelDesc);
                            if (!val || (*val)->price->price > price->price) {
                                opendata_result_t *item = malloc(sizeof(opendata_result_t));
                                if(!item) continue;
                                item->station = station;
                                item->price = price;
                                map_set(&map, price->fuelDesc, item);
                                log_debug(u8"Aggiunta categoria alla mappa di ricerca: %s", price->fuelDesc);
                            }
                        } else {
                            printf( //
                                   u8"\t%s, %.3f euro %s\n", //
                                   price->fuelDesc, //
                                   price->price, //
                                   price->self == 0 ? "non servito" : "servito" //
                            );
                        }
                    }
                    price = price->next;
                }
            }
            station = station->next;
        }
    }

    const char *key;
    map_iter_t iter = map_iter(&map);

    while ((key = map_next(&map, &iter))) {
        opendata_result_t **val = map_get(&map, key);
        printf( //
            u8"Miglior prezzo %s -> [%s / %s] %s (%s) / %.3f euro %s\n", //
            key, //
            (*val)->station->town, //
            (*val)->station->type, //
            (*val)->station->name, //
            (*val)->station->address, //
            (*val)->price->price, //
            ((*val)->price->self == 0) ? "non servito" : "servito"
        );
    }

#ifdef NO_CACHE
    if (remove(anagrafiaFilename) != 0)
        log_error("Impossibile eliminare il file %s", anagrafiaFilename);

    if (remove(priceFilename) != 0)
        log_error("Impossibile eliminare il file %s", priceFilename);
#endif // NO_CACHE

    map_deinit(&map);

    free(anagrafiaFilename);
    free(priceFilename);
    freeStationList(stations);
    freePriceList(prices);
    free(stations);
    free(prices);

    return EXIT_SUCCESS;
}
