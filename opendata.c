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

#include "opendata.h"

static char* strToLower(const char* str) {
    if(!str)
        return NULL;
    char* result = dmt_malloc(sizeof(char) * strlen(str));
    if(result) {
        for(size_t i = 0; i <= strlen(str); i++) result[i] = tolower(str[i]);
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
                log_error("curl_easy_perform() failed: %s", curl_easy_strerror(res));

            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
            if(http_code != 200)
                log_error("I'azione e\' finita in errore %d", http_code);
            fclose(file);
        }
    }

    curl_easy_cleanup(curl);
    return (res == CURLE_OK && http_code == 200) ? true : false;
}

station_t* stationFinder(char* filename, char* separator, bool header, char* query) {
    station_t *head = (station_t*) dmt_malloc(sizeof(station_t));
    if(!head)
        return NULL;
    head->id = 0;
    head->name = NULL;
    head->type = NULL;
    head->address = NULL;
    head->town = NULL;
    head->next = NULL;
    CsvParser *csvparser = CsvParser_new(filename, separator, header);
    if(csvparser) {
        CsvRow *row = NULL;
        station_t *current = head;
        while ((row = CsvParser_getRow(csvparser))) {
            const char **rowFields = CsvParser_getFields(row);
            if(CsvParser_getNumFields(row) == 10) {
                if(strcasecmp(rowFields[6], query) == 0) {
                    current->id = atoi(rowFields[0]);
                    current->name = strdup(rowFields[1]);
                    current->type = strdup(rowFields[2]);
                    current->address = strdup(rowFields[5]);
                    current->town = strdup(rowFields[6]);
                    current->next = (station_t *) dmt_malloc(sizeof(station_t));
                    current->next->next = NULL;
                    current = current->next;
                }
            }
            if(row) CsvParser_destroy_row(row);
        }
        if(csvparser) CsvParser_destroy(csvparser);
    }
    return head;
}

price_t* priceFinder(char* filename, char* separator, bool header, station_t* list, char* type) {
    price_t *head = (price_t*) dmt_malloc(sizeof(price_t));
    if(!head)
        return NULL;
    head->next = NULL;
    CsvParser *csvparser = CsvParser_new(filename, separator, header);
    if(csvparser) {
        CsvRow *row = NULL;
        price_t *current = head;
        while ((row = CsvParser_getRow(csvparser))) {
            const char **rowFields = CsvParser_getFields(row);
            if(CsvParser_getNumFields(row) == 5) {
                station_t *tmp = list;
                char *lowerType = strToLower(type);
                while (tmp->next != NULL) {
                    char *name = strdup(rowFields[1]);
                    if(tmp->id == atoi(rowFields[0]) && (!type || strstr(strToLower(name), lowerType) != NULL)) {
                        current->id = atoi(rowFields[0]);
                        current->fuelDesc = strdup(rowFields[1]);
                        current->price = atof(rowFields[2]);
                        current->self = atoi(rowFields[3]);
                        current->lastUpdate = strdup(rowFields[4]);
                        current->next = (price_t *) dmt_malloc(sizeof(price_t));
                        current->next->next = NULL;
                        current = current->next;
                    }
                    tmp = tmp->next;
                }
                dmt_free(lowerType);
            }
            if(row) CsvParser_destroy_row(row);
        }
        if(csvparser) CsvParser_destroy(csvparser);
    }
    return head;
}

void freeStationList(station_t* list) {
    if(list->next) {
        freeStationList(list->next);
    }
    dmt_free(list);
}

void freePriceList(price_t* list) {
    if(list->next) {
        freePriceList(list->next);
    }
    dmt_free(list);
}

#ifdef COLOR
bool is_old_data(const char* data) {

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

    return difftime(mktime(localtime(&now)), mktime(&param)) > 259200; // 3 deys
}
#endif // COLOR


