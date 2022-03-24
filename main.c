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

#include "main.h"

int main(int argn, char* argv[]) {

    log_set_level(LOG_ERROR);

    char* query = NULL;
    char* type = NULL;
    bool searchonly = false;
    bool ignorecache = false;
    bool ignoreold = false;

    char option = '\0';
    while((option = getopt_long(argn, argv, "q:t:suovh", long_options, NULL)) != -1) {
        switch(option) {
        case 'q':
            if (optarg) query = optarg;
            break;
        case 't':
            if (optarg) type = optarg;
            break;
        case 's':
            searchonly = true;
            break;
        case 'u':
            ignorecache = true;
            break;
        case 'o':
            ignoreold = true;
            break;
        case 'v':
            log_set_level(LOG_DEBUG);
            break;
        case 'h':
            fprintf(stdout, "%s %s\n"
                    "\n"
                    "Copyright (C) 2020-2022 Marco Magliano Free Software Foundation, Inc.\n"
                    "This is free software; see the source for copying conditions.\n"
                    "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n"
                    "PARTICULAR PURPOSE.\n"
                    "\n"
                    "Buildtime: %s - %s\n\n", "cliFuel", PROGRAM_VERSION, __DATE__, __TIME__);
#ifdef FILE_DOWNLOAD
            fprintf(stdout, "\t-With automatic file download feature\n");
#endif // FILE_DOWNLOAD
#ifdef ANIMATION
            fprintf(stdout, "\t-With progress animation feature\n");
#endif // ANIMATION
#ifdef COLOR
            fprintf(stdout, "\t-With color on price items feature\n");
#endif // COLOR
#ifdef DEBUG
            fprintf(stdout, "\t-With debug extra output feature\n");
#endif // DEBUG
#ifdef NO_CACHE
            fprintf(stdout, "\t-With no file cache feature\n");
#endif // NO_CACHE
            fprintf(stdout, "\n");
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
        printf("Nessuna query specificata, impossibile proseguire\n");
        exit(EXIT_SUCCESS);
    }

    if(type) {
        log_debug("Filtro tipologia carburante: %s", type);
    }

    if(searchonly) {
        log_debug("Modalita' di sola ricerca impianti");
    }

    if(ignorecache) {
        log_debug("Modalita' ignore cache attiva");
#ifdef NO_CACHE
        log_warn("Tipo di build gia' NO_CACHE");
#endif // NO_CACHE
    }

    if(ignoreold) {
        log_debug("Modalita' ignore old attiva");
    }

    char *cacheDir = get_file_path(argv[0], NULL);

    struct stat st = {0};
    if (stat(cacheDir, &st) == -1) {
#ifdef _WIN32
        mkdir(cacheDir);
#else
        mkdir(cacheDir, 0700);
#endif
    }

    dmt_free(cacheDir);

#ifdef ANIMATION
    pthread_t thread;
    statusbar *animation = statusbar_new("Elaborazione in corso...");
    int rc = pthread_create(&thread, NULL, progress, animation);
    if (rc) {
        log_error("Errore: impossibile creare il thread, %d", rc);
        exit(EXIT_FAILURE);
    }
#endif // ANIMATION

    char* anagrafiaFilename = get_file_path(argv[0], ANAGRAFIA_IMPIANTI_FILE);
    char* priceFilename = get_file_path(argv[0], PREZZI_FILE);

    if(!anagrafiaFilename || !priceFilename) exit(EXIT_FAILURE);

    log_debug("File anagrafia: %s", anagrafiaFilename);
    log_debug("File prezzi: %s", priceFilename);

    struct tm *timenow;
    time_t now = time(NULL);
    timenow = gmtime(&now);

    char* patternAnagrafiaFilename = strdup(anagrafiaFilename);
    char* patternPriceFilename = strdup(priceFilename);

    strftime(anagrafiaFilename, strlen(anagrafiaFilename), patternAnagrafiaFilename, timenow);
    strftime(priceFilename, strlen(priceFilename), patternPriceFilename, timenow);

    free(patternAnagrafiaFilename);
    free(patternPriceFilename);

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

    // Se i file ancora non esistono...
    if(!(access(anagrafiaFilename, F_OK ) != -1) || !(access(priceFilename, F_OK ) != -1)) {
        log_error("File non presenti, impossibile proseguire");
        exit(EXIT_FAILURE);
    }

    station_t* stations = stationFinder(anagrafiaFilename, FILE_SEPARATOR, FILE_HEADER, query);
    if(!stations) {
        log_error("Impossibile indicizzare le stazioni oppure nessun risultato trovato");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    if(stations) {
        station_t *tmp = stations;
        while (tmp) {
            log_debug(u8"%d - %s - %s - %s\n", tmp->id, tmp->name, tmp->address, tmp->town);
            tmp = tmp->next;
        }
    }
#endif // DEBUG

    price_t* prices = priceFinder(priceFilename, FILE_SEPARATOR, FILE_HEADER, stations, type);
    if(!prices) {
        log_error("Impossibile indicizzare i prezzi oppure nessun risultato trovato");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    if(prices) {
        price_t *tmp = prices;
        while (tmp) {
            log_debug(u8"%s, %.3f euro %s (%s)\n", tmp->fuelDesc, tmp->price, tmp->self == NOT_SELF ? "non servito" : "servito", tmp->lastUpdate);
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

    if(stations) {
        station_t *station = stations;
        while (station) {
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
                while (price) {
                    if(price->id == station->id) {
                        if(!(ignoreold && price->is_old == OLD)) {
                            if(!searchonly) {
                                opendata_result_t **val = map_get(&map, price->fuelDesc);
                                if (!val || (*val)->price->price > price->price) {
                                    if(val) dmt_free(*val); // Elemento precedente già in mappa
                                    opendata_result_t *item = dmt_malloc(sizeof(opendata_result_t));
                                    if(!item) continue;
                                    item->station = station;
                                    item->price = price;
                                    map_set(&map, price->fuelDesc, item);
                                    log_debug(u8"Aggiunta categoria alla mappa di ricerca: %s", price->fuelDesc);
                                }
                            } else {
#ifdef COLOR
                                char *alert = make_alert(price);
#endif // COLOR
                                printf( //
                                    u8"\t%s, %.3f euro %s (dato del %s)\n", //
                                    price->fuelDesc, //
                                    price->price, //
                                    price->self == NOT_SELF ? "non servito" : "servito", //
#ifdef COLOR
                                    (alert) ? alert : "" //
#else
                                    price->lastUpdate //
#endif // COLOR
                                );
#ifdef COLOR
                                if(alert) dmt_free(alert);
#endif // COLOR
                            }
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
        if(!(ignoreold && (*val)->price->is_old == OLD)) {
#ifdef COLOR
            char *alert = make_alert((*val)->price);
#endif // COLOR
            printf( //
                u8"Miglior prezzo %s -> [%s / %s] %s (%s) / %.3f euro %s (ultimo aggiornamento il %s)\n", //
                key, //
                (*val)->station->town, //
                (*val)->station->type, //
                (*val)->station->name, //
                (*val)->station->address, //
                (*val)->price->price, //
                ((*val)->price->self == NOT_SELF) ? "non servito" : "servito", //
#ifdef COLOR
                (alert) ? alert : "" //
#else
                (*val)->price->lastUpdate //
#endif // COLOR
            );
#ifdef COLOR
            if(alert) dmt_free(alert);
#endif // COLOR
        }
        dmt_free(*val);
    }

#ifdef COLOR
    fprintf(stdout, "\n\n");
    fprintf(stdout, "Legenda dei colori:\n");
    fprintf(stdout, "\t-%s\n", COLOR_START_PATTERN(32) u8"Dato di oggi" COLOR_END_PATTERN);
    fprintf(stdout, "\t-%s\n", COLOR_START_PATTERN(93) u8"Dato di uno, massimo due giorni" COLOR_END_PATTERN);
    if(!ignoreold) {
        fprintf(stdout, "\t-%s\n", COLOR_START_PATTERN(31) u8"Dato con più di tre giorni" COLOR_END_PATTERN);
    }
    fprintf(stdout, "\n\n");
#endif // COLOR

#ifdef NO_CACHE
    if (remove(anagrafiaFilename) != 0)
        log_error("Impossibile eliminare il file %s", anagrafiaFilename);

    if (remove(priceFilename) != 0)
        log_error("Impossibile eliminare il file %s", priceFilename);
#endif // NO_CACHE

    dmt_free(anagrafiaFilename);
    dmt_free(priceFilename);

    map_deinit(&map);

    freeStationList(stations);
    freePriceList(prices);

#ifdef DEBUG
    dmt_dump(stdout);
#endif // DEBUG

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
#if defined _WIN32 || defined _WIN64
            Sleep(100);
#else
            sleep(0.50);
#endif
        }
    }
    return NULL;
}
#endif // ANIMATION

static char* get_file_path(const char* dir, const char* filename) {

    if(!dir) {
        perror("arument = null");
        return NULL;
    }

    char *tmp = strdup(dir);

    char *path = dmt_calloc(PATH_MAX, sizeof(char));
    if(!path) {
        perror("path = null");
        return NULL;
    }
    snprintf(path, PATH_MAX, "%s/" CACHE_DIR "/%s", dirname(tmp), filename ? filename : "");

    free(tmp);

    return path;
}

static void helpme() {
    printf("Parametri:\n\n"
           "\t-q --query --> Comune di ricerca oppure id impianto usando es: \"-q id:34974\" \n"
           "\t-t --type --> Filtro tipologia carburante\n"
           "\t-s --search-only --> Ricerca semplice\n"
           "\t-u --ignore-cache --> Ignora la cache\n"
           "\t-o --ignore-old --> Ignora i record non aggiornati di recente\n"
           "\t-v --verbose --> Log verboso\n"
           "\t-h --help --> Visualizza questo aiuto\n");
}
