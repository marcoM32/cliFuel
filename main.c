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

    char* query = NULL;
    char* type = NULL;
    bool searchonly = false;
    bool ignorecache = false;

    char option = '\0';
    while((option = getopt_long(argn, argv, "q:t:suvh", long_options, NULL)) != -1) {
        switch(option) {
        case 'q':
            if (optarg)
                query = optarg;
            break;
        case 't':
            if (optarg)
                type = optarg;
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
                    "Buildtime: %s - %s\n\n", "cliFuel", PROGRAM_VERSION, __DATE__, __TIME__);
                    helpme();
            exit(EXIT_SUCCESS);
            break;
        case '?':
            fprintf(stdout, "Comando sconosciuto: %c\n", optopt);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if(!query) {
        printf("Nessuna query specificata, impossibile proseguire");
        exit(EXIT_SUCCESS);
    }

    struct stat st = {0};
    if (stat(CACHE_DIR, &st) == -1) {
#ifdef _WIN32
        mkdir(CACHE_DIR);
#else
        mkdir(CACHE_DIR, 0700);
#endif
    }

#ifdef ANIMATION
    pthread_t thread;
    statusbar *animation = statusbar_new("Elaborazione in corso...");
    int rc = pthread_create(&thread, NULL, progress, animation);
    if (rc) {
        log_error("Errore: impossibile creare il thread, %d", rc);
        exit(EXIT_FAILURE);
    }
#endif // ANIMATION

    char* anagrafiaFilename = strdup(FILENAME(CACHE_DIR, ANAGRAFIA_IMPIANTI_FILE));
    char* priceFilename = strdup(FILENAME(CACHE_DIR, PREZZI_FILE));

    if(!anagrafiaFilename || !priceFilename) exit(EXIT_FAILURE);

    log_debug("File anagrafia: %s", anagrafiaFilename);
    log_debug("File prezzi: %s", priceFilename);

    struct tm *timenow;
    time_t now = time(NULL);
    timenow = gmtime(&now);
    strftime(anagrafiaFilename, strlen(FILENAME(CACHE_DIR, ANAGRAFIA_IMPIANTI_FILE)) + 1, FILENAME(CACHE_DIR, ANAGRAFIA_IMPIANTI_FILE), timenow);
    strftime(priceFilename, strlen(FILENAME(CACHE_DIR, PREZZI_FILE)) + 1, FILENAME(CACHE_DIR, PREZZI_FILE), timenow);

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

    price_t* prices = priceFinder(priceFilename, FILE_SEPARATOR, FILE_HEADER, stations, type);
#ifdef DEBUG
    if(prices != NULL) {
        price_t *tmp = prices;
        while (tmp->next != NULL) {
            log_debug(u8"%s, %.3f euro %s (%s)\n", tmp->fuelDesc, tmp->price, tmp->self == 0 ? "non servito" : "servito", tmp->lastUpdate);
            tmp = tmp->next;
        }
    }
#endif // DEBUG

#ifdef ANIMATION
    pthread_cancel(thread);
    statusbar_finish(animation);
#endif // ANIMATION

    opendata_map_t map;
    map_init(&map);

    if(stations != NULL) {
        station_t *station = stations;
        while (station->next != NULL) {
            if(searchonly) {
                printf( //
                    u8"* %s - %s - %s\n", //
                    station->name, //
                    station->address, //
                    station->town //
                );
            }
            if(prices != NULL) {
                price_t *price = prices;
                while (price->next != NULL) {
                    if(price->id == station->id) {
                        if(!searchonly) {
                            opendata_result_t **val = map_get(&map, price->fuelDesc);
                            if (!val || (*val)->price->price > price->price) {
                                opendata_result_t *item = dmt_malloc(sizeof(opendata_result_t));
                                if(!item) continue;
                                item->station = station;
                                item->price = price;
                                map_set(&map, price->fuelDesc, item);
                                log_debug(u8"Aggiunta categoria alla mappa di ricerca: %s", price->fuelDesc);
                            }
                        } else {
                            printf( //
                                u8"\t%s, %.3f euro %s (dato del \x1B[0m %s \e[0m)\n", //
                                price->fuelDesc, //
                                price->price, //
                                price->self == 0 ? "non servito" : "servito", //
                                price->lastUpdate //
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

#ifdef COLOR
        char *pattern = dmt_malloc(sizeof(char) * strlen((*val)->price->lastUpdate) + strlen(COLOR_START_PATTERN) +  strlen(COLOR_END_PATTERN));
        if(!pattern)
            return EXIT_FAILURE;
        sprintf(pattern, is_old_data((*val)->price->lastUpdate) ? COLOR_START_PATTERN "%s" COLOR_END_PATTERN : "%s", (*val)->price->lastUpdate);
#endif // COLOR

        printf( //
            u8"Miglior prezzo %s -> [%s / %s] %s (%s) / %.3f euro %s (ultimo aggiornamento il %s)\n", //
            key, //
            (*val)->station->town, //
            (*val)->station->type, //
            (*val)->station->name, //
            (*val)->station->address, //
            (*val)->price->price, //
            ((*val)->price->self == 0) ? "non servito" : "servito", //
#ifdef COLOR
            pattern //
#else
            (*val)->price->lastUpdate //
#endif // COLOR
        );
#ifdef COLOR
        dmt_free(pattern);
#endif // COLOR
        dmt_free(*val);
    }

#ifdef NO_CACHE
    if (remove(anagrafiaFilename) != 0)
        log_error("Impossibile eliminare il file %s", anagrafiaFilename);

    if (remove(priceFilename) != 0)
        log_error("Impossibile eliminare il file %s", priceFilename);
#endif // NO_CACHE

    map_deinit(&map);

    freeStationList(stations);
    freePriceList(prices);

    return EXIT_SUCCESS;
}

#ifdef ANIMATION
static void * progress(void *argc) {
    statusbar *animation = (statusbar*) argc;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if(animation) {
        while(true) {
            statusbar_inc(animation);
#ifdef _WIN32
            Sleep(100);
#else
            sleep(0.50);
#endif
        }
    }
    return NULL;
}
#endif // ANIMATION

static void helpme() {
  printf("Parametri:\n\n"
	"\t-q --query --> Comune di ricerca\n"
	"\t-s --search-only --> Ricerca semplice\n"
    "\t-u --ignore-cache --> Ignora la cache\n"
    "\t-v --verbose --> Log verboso\n"
    "\t-h --help --> Visualizza questo aiuto\n");
}
