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

#include "opendata.h"

static bool strStartsWith(const char *a, const char *b) {
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}

static char* strCopy(const char* str) {
    if(!str)
        return NULL;
    size_t len = strlen(str) + 1;
    char* result = dmt_malloc(sizeof(char) * len);
    memcpy(result, str, len);
    result[len - 1] = '\0';
    return result;
}

static char* strToLower(const char* str) {
    char* result = strCopy(str);
    if(result) {
        for (size_t i = 0; result[i] != '\0'; i++) if(isupper(result[i])) result[i] = tolower(result[i]);
        return result;
    }
    return NULL;
}

bool download(const char* url, const char* target) {
    long http_code = 0;
    CURL *curl = NULL;
    CURLcode res = CURLE_OK;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if(!curl)
        return false;

#ifdef DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    if(url && target) {

#ifdef DEBUG
        log_debug("URL: %s", url);
        log_debug("Target: %s", target);
#endif // DEBUG

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

        FILE* file = fopen(target,"wb");
        if(file) {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
                log_error("curl_easy_perform() fallita: %s", curl_easy_strerror(res));

            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

            fclose(file);

            if(http_code != 200) {
                log_error("Connessione con endpoint ko %d", http_code);
                remove(target);
            }
        }
    }

    curl_easy_cleanup(curl);
    return (res == CURLE_OK && http_code == 200) ? true : false;
}

station_t* stationFinder(char* filename, char* separator, bool header, char* query) {
    station_t *head = NULL;
    CsvParser *csvparser = CsvParser_new(filename, separator, header);
    if(csvparser) {
        CsvRow *row = NULL;
        while ((row = CsvParser_getRow(csvparser))) {
            const char **rowFields = CsvParser_getFields(row);
            if(CsvParser_getNumFields(row) == 10) {
                int id = -1;
                if(strStartsWith(query, QUERY_PREFIX_ID)) {
                    id = atoi(query + +strlen(QUERY_PREFIX_ID));
                }
                int stationId = atoi(rowFields[0]);
                if((id == stationId) || strcasecmp(rowFields[6], query) == 0) {
                    station_t *current = (station_t *) dmt_malloc(sizeof(station_t));
                    current->id = stationId;
                    current->name = strCopy(rowFields[1]);
                    current->type = strCopy(rowFields[2]);
                    current->address = strCopy(rowFields[5]);
                    current->town = strCopy(rowFields[6]);
                    current->next = NULL;
                    if(!head) {
                        head = current;
                    } else {
                        station_t *stmp = head;
                        while(stmp->next) {
                            stmp = stmp->next;
                        }
                        stmp->next = current;
                    }
                }
            }
            CsvParser_destroy_row(row);
        }
        CsvParser_destroy(csvparser);
    }
    return head;
}

price_t* priceFinder(char* filename, char* separator, bool header, station_t* list, char* type) {
    price_t *head = NULL;
    CsvParser *csvparser = CsvParser_new(filename, separator, header);
    if(csvparser) {
        CsvRow *row = NULL;
        char *lowerType = strToLower(type);
        while ((row = CsvParser_getRow(csvparser))) {
            const char **rowFields = CsvParser_getFields(row);
            if(CsvParser_getNumFields(row) == 5) {
                station_t *tmp = list;
                while (tmp) {
                    char *lowerName = strToLower(rowFields[1]);
                    if(tmp->id == atoi(rowFields[0]) && (!type || strstr(lowerName, lowerType) != NULL)) {
                        price_t *current = (price_t *) dmt_malloc(sizeof(price_t));
                        current->id = atoi(rowFields[0]);
                        current->fuelDesc = strCopy(rowFields[1]);
                        current->price = atof(rowFields[2]);
                        current->self = atoi(rowFields[3]);
                        current->lastUpdate = strCopy(rowFields[4]);
                        current->is_old = is_old_data(current->lastUpdate);
                        current->next = NULL;
                        if(!head) {
                            head = current;
                        } else {
                            price_t *ptmp = head;
                            while(ptmp->next) {
                                ptmp = ptmp->next;
                            }
                            ptmp->next = current;
                        }
                    }
                    if(lowerName) dmt_free(lowerName);
                    tmp = tmp->next;
                }
            }
            CsvParser_destroy_row(row);
        }
        if(lowerType) dmt_free(lowerType);
        CsvParser_destroy(csvparser);
    }
    return head;
}

void freeStationList(station_t* list) {
    if(list->next) {
        freeStationList(list->next);
        list->next = NULL;
    }

    if(list->address) {
        dmt_free(list->address);
        list->address = NULL;
    }

    if(list->name) {
        dmt_free(list->name);
        list->name = NULL;
    }

    if(list->town) {
        dmt_free(list->town);
        list->town = NULL;
    }

    if(list->type) {
        dmt_free(list->type);
        list->type = NULL;
    }

    dmt_free(list);
    list = NULL;
}

void freePriceList(price_t* list) {
    if(list->next) {
        freePriceList(list->next);
        list->next = NULL;
    }

    if(list->fuelDesc) {
        dmt_free(list->fuelDesc);
        list->fuelDesc = NULL;
    }

    if(list->lastUpdate) {
        dmt_free(list->lastUpdate);
        list->lastUpdate = NULL;
    }

    dmt_free(list);
    list = NULL;
}

enum item_age is_old_data(const char* data) {

    if(!data)
        return false;

    struct tm param = {0};
    char *s = strptime(data, "%d/%m/%Y %H:%M:%S", &param);
    if (s == NULL) {
        log_error("strptime() failed: %s", data);
        return false;
    }

    time_t now = time(NULL);
    if (now == -1) {
        log_error("time(NULL) failed");
        return false;
    }

    size_t diff = difftime(mktime(localtime(&now)), mktime(&param));
    if(diff <= 86400) {
        return NEW; // 1 day
    } else if(diff >= 172800 && diff <= 259200) {
        return OK;  // 2-3 day
    } else {
        return OLD; // > 3 day
    }
}

#ifdef COLOR
char* make_alert(const price_t *price) {
    if(!price) return NULL;
    char *pattern = dmt_malloc(sizeof(char) * strlen(price->lastUpdate) + strlen(COLOR_START_PATTERN(100)) /* !!! Pericoloso !!! */ +  strlen(COLOR_END_PATTERN) + 1); // + \0
    if(!pattern) return NULL;
    switch(price->is_old) {
    case NEW:
        sprintf(pattern, COLOR_START_PATTERN(32) "%s" COLOR_END_PATTERN, price->lastUpdate);
        break;
    case OK:
        sprintf(pattern, COLOR_START_PATTERN(93) "%s" COLOR_END_PATTERN, price->lastUpdate);
        break;
    case OLD:
        sprintf(pattern, COLOR_START_PATTERN(31) "%s" COLOR_END_PATTERN, price->lastUpdate);
        break;
    default:
        pattern = NULL;
    }
    return pattern;
}
#endif // COLOR


