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

#ifdef HTML_OUTPUT
    html_fprintf(stdout, HTML_TAG_OPEN_PAGE, NULL, NEW_LINE);
    atexit(on_exit_close_tag);
#endif

    if(argn <= 1) helpme(), exit(EXIT_SUCCESS);

    log_set_level(LOG_ERROR);

    char* query = NULL;
    char* type = NULL;
    char* custompath = NULL;
    bool searchonly = false;
    bool ignorecache = false;
    bool ignoreold = false;

    char option = '\0';
    while((option = getopt_long(argn, argv, "q:t:p:suovh", long_options, NULL)) != -1) {
        switch(option) {
        case 'q':
            if (optarg) query = optarg;
            break;
        case 't':
            if (optarg)
                type = optarg;
            break;
        case 'p':
            if (optarg) custompath = optarg;
            if(!custompath || !is_directory(custompath)) {
                fprintf(stdout, u8"Il percorso indicato con -p (--path) non è una directory valida" NEW_LINE);
                exit(EXIT_FAILURE);
            }
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
            helpme();
            exit(EXIT_SUCCESS);
            break;
        case '?':
            fprintf(stdout, "Comando sconosciuto: %c" NEW_LINE, optopt);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if(!query) {
        fprintf(stdout, "Nessuna query specificata, impossibile proseguire" NEW_LINE);
        exit(EXIT_SUCCESS);
    }

    if(type) {
        log_debug("Filtro tipologia carburante: %s", type);
    }

    if(custompath) {
        log_debug("Percorso cache castomizzato: %s", custompath);
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

#ifdef ANIMATION
    pthread_t thread;
    statusbar *animation = statusbar_new("Elaborazione in corso...");
    int rc = pthread_create(&thread, NULL, progress, animation);
    if (rc) {
        log_error("Errore: impossibile creare il thread, %d", rc);
        exit(EXIT_FAILURE);
    }
#endif // ANIMATION

    log_debug("Indirizzo binario: %s", argv[0]);

    char* anagrafiaFilename = get_file_path(custompath ? custompath : argv[0], ANAGRAFIA_IMPIANTI_FILE);
    char* priceFilename = get_file_path(custompath ? custompath : argv[0], PREZZI_FILE);

    if(!anagrafiaFilename || !priceFilename) exit(EXIT_FAILURE);

    log_debug("File anagrafia ipotetico: %s", anagrafiaFilename);
    log_debug("File prezzi ipotetico: %s", priceFilename);

    struct tm *timenow;
    time_t now = time(NULL);
    timenow = gmtime(&now);

    char* patternAnagrafiaFilename = strdup(anagrafiaFilename);
    char* patternPriceFilename = strdup(priceFilename);

    strftime(anagrafiaFilename, PATH_MAX, patternAnagrafiaFilename, timenow);
    strftime(priceFilename, PATH_MAX, patternPriceFilename, timenow);

    log_debug("File anagrafia definitivo: %s", anagrafiaFilename);
    log_debug("File prezzi definitivo: %s", priceFilename);

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
            log_debug(u8"%d - %s - %s - %s", tmp->id, tmp->name, tmp->address, tmp->town);
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
            log_debug(u8"%s, %.3f euro %s (%s)", tmp->fuelDesc, tmp->price, tmp->self == NOT_SELF ? "non servito" : "servito", tmp->lastUpdate);
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
                fprintf(stdout, //
                        u8"* %s - %s - %s" NEW_LINE, //
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
                                fprintf(stdout, //
                                        TAB u8"%s, %.3f euro %s (dato del %s)" NEW_LINE, //
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
            fprintf(stdout, //
                    u8"Miglior prezzo %s -> [%s / %s] %s (%s) / %.3f euro %s (ultimo aggiornamento il %s)" NEW_LINE, //
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
    fprintf(stdout, NEW_LINE NEW_LINE);
    fprintf(stdout, "Legenda dei colori:" NEW_LINE);
    fprintf(stdout, TAB "-%s" NEW_LINE, COLOR_START_PATTERN(32) u8"Dato di oggi" COLOR_END_PATTERN);
    fprintf(stdout, TAB "-%s" NEW_LINE, COLOR_START_PATTERN(93) u8"Dato di uno, massimo due giorni" COLOR_END_PATTERN);
    if(!ignoreold) {
        fprintf(stdout, TAB "-%s" NEW_LINE, COLOR_START_PATTERN(31) u8"Dato con più di tre giorni" COLOR_END_PATTERN);
    }
    fprintf(stdout, NEW_LINE NEW_LINE);
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
    if(tmp) {
        char *path = dmt_calloc(PATH_MAX, sizeof(char));
        if(!path) {
            perror("path = null");
            return NULL;
        }
        snprintf(path, PATH_MAX - 1, ((tmp[strlen(tmp) - 1] == '/') ? "%s" CACHE_DIR : "%s/" CACHE_DIR),
                 is_directory(tmp) ? tmp : dirname(tmp));

        if(!is_directory(path)) create_directory(path);

        snprintf(path + strlen(path), PATH_MAX - 1, "/%s", filename ? filename : "");

        free(tmp);
        return path;
    }
    perror("tmp = null");
    return NULL;
}

static int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

static void create_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
#ifdef _WIN32
        mkdir(path);
#else
        mkdir(path, 0700);
#endif
    }
}

#ifdef HTML_OUTPUT
static void on_exit_close_tag() {
    html_fprintf(stdout, NULL, HTML_TAG_CLOSE_PAGE, NEW_LINE);

}

static void html_fprintf(FILE * _File, const char * opentag, const char * closetag, const char * _Format, ...) {
    va_list args;
    char *result = dmt_malloc(((opentag) ? strlen(opentag) : 0) + ((closetag) ? strlen(closetag) : 0) + strlen(_Format) + 1);
    if (result) {
        va_start(args, _Format);
        int pos = 0;
        if(opentag)
            pos += sprintf(&result[pos], "%s", opentag);
        pos += sprintf(&result[pos], "%s", _Format);
        if(closetag)
            pos += sprintf(&result[pos], "%s", closetag);
        vfprintf(_File, result, args);
        va_end(args);
        dmt_free(result);
    }
}
#endif // HTML_OUTPUT

static void helpme() {
    fprintf(stdout, "%s %s" NEW_LINE
            NEW_LINE
            "Copyright (C) 2020-2022 Marco Magliano Free Software Foundation, Inc." NEW_LINE
            "This is free software; see the source for copying conditions." NEW_LINE
            "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A" NEW_LINE
            "PARTICULAR PURPOSE." NEW_LINE
            NEW_LINE
            "Buildtime: %s - %s" NEW_LINE NEW_LINE, PROGRAM_NAME, PROGRAM_VERSION, __DATE__, __TIME__);
#ifdef FILE_DOWNLOAD
    fprintf(stdout, TAB "-With automatic file download feature" NEW_LINE);
#endif // FILE_DOWNLOAD
#ifdef ANIMATION
    fprintf(stdout, TAB "-With progress animation feature" NEW_LINE);
#endif // ANIMATION
#ifdef COLOR
    fprintf(stdout, TAB "-With color on price items feature" NEW_LINE);
#endif // COLOR
#ifdef DEBUG
    fprintf(stdout, TAB "-With debug extra output feature" NEW_LINE);
#endif // DEBUG
#ifdef NO_CACHE
    fprintf(stdout, TAB "-With no file cache feature" NEW_LINE);
#endif // NO_CACHE
#ifdef HTML_OUTPUT
    fprintf(stdout, TAB "-With html output format" NEW_LINE);
#endif // HTML_OUTPUT
    fprintf(stdout, NEW_LINE);
    fprintf(stdout, "Parametri:" NEW_LINE NEW_LINE
            TAB "-q --query --> Comune di ricerca oppure id impianto usando es: \"-q %s34974 || -q %sRO || -q %sroma,bologna\"" NEW_LINE
            TAB "-t --type --> Filtro tipologia carburante" NEW_LINE
            TAB "-p --path --> Percorso cache applicativa" NEW_LINE
            TAB "-s --search-only --> Ricerca semplice" NEW_LINE
            TAB "-u --ignore-cache --> Ignora la cache" NEW_LINE
            TAB "-o --ignore-old --> Ignora i record non aggiornati di recente" NEW_LINE
            TAB "-v --verbose --> Log verboso" NEW_LINE
            TAB "-h --help --> Visualizza questo aiuto" NEW_LINE, QUERY_PREFIX_ID, QUERY_PREFIX_PROV, QUERY_PREFIX_LIST);
}
