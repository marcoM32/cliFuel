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
    station_t *head = (station_t*) malloc(sizeof(station_t));
    if(!head)
        return NULL;
    head->next = NULL;
    CsvParser *csvparser = CsvParser_new(filename, separator, header);
    if(csvparser) {
        CsvRow *row = NULL;
        station_t *current = head;
        while ((row = CsvParser_getRow(csvparser))) {
            const char **rowFields = CsvParser_getFields(row);
            if(CsvParser_getNumFields(row) == 10) {
                if(stricmp(rowFields[6], query) == 0) {
                    current->id = atoi(rowFields[0]);
                    current->name = strdup(rowFields[1]);
                    current->type = strdup(rowFields[2]);
                    current->address = strdup(rowFields[5]);
                    current->town = strdup(rowFields[6]);
                    current->next = (station_t *) malloc(sizeof(station_t));
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

price_t* priceFinder(char* filename, char* separator, bool header, station_t* list) {
    price_t *head = (price_t*) malloc(sizeof(price_t));
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
                while (tmp->next != NULL) {
                    if(tmp->id == atoi(rowFields[0])) {
                        current->id = atoi(rowFields[0]);
                        char* fuelDesc_value = malloc((sizeof(char) * strlen(rowFields[1])) + 1);
                        strcpy(fuelDesc_value, rowFields[1]);
                        current->fuelDesc = fuelDesc_value;
                        current->price = atof(rowFields[2]);
                        current->self = atoi(rowFields[3]);
                        current->next = (price_t *) malloc(sizeof(price_t));
                        current->next->next = NULL;
                        current = current->next;
                    }
                    tmp = tmp->next;
                }
            }
            if(row) CsvParser_destroy_row(row);
        }
        if(csvparser) CsvParser_destroy(csvparser);
    }
    return head;
}

void freeStationList(station_t* list) {
    if(list->next)
        freeStationList(list->next);
    free(list);
}

void freePriceList(price_t* list) {
    if(list->next)
        freePriceList(list->next);
    free(list);
}
