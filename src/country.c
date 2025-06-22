#include <stdio.h>
#include <string.h>
#include "../include/country.h"
#include "../include/utils.h"

/*Return name of country*/
char* country_getName(Country *country) {

    return country->name;
}

/*Compare two countries based on their name*/
int country_compare(void* data1, void* data2){

    return strcmp( ((Country*) data1)->name , ((Country*) data2)->name );
}

int mystrcmp(void *data1, void *data2) {

    return strcmp((char*)data1,(char*)data2);
}

/*Compare two countries based on their name*/
int countryFolders_compare(void* data1, void* data2){

    return strcmp( ((CountryFolder*) data1)->name , ((CountryFolder*) data2)->name );
}

int monitoredCountry_compare(void *str1, void *str2) {

    char countryName[20];
    strcpy(countryName,((MonitoredCountry*)str1)->name);
    capitalize(countryName);

    return strcmp(countryName,((MonitoredCountry*)str2)->name);
}

int monitoredCountry_compareNonCap(void *str1, void *str2) {

    char countryName[20];
    strcpy(countryName,((MonitoredCountry*)str1)->name);

    return strcmp(countryName,((MonitoredCountry*)str2)->name);
}