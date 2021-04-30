#include <stdio.h>
#include <string.h>
#include "country.h"

/*Return name of country*/
char* country_getName(Country *country) {

    return country->name;
}

/*Compare two countries based on their name*/
int country_compare(void* data1, void* data2){

    return strcmp( ((Country*) data1)->name , ((Country*) data2)->name );
}

int monitoredCountry_compare(void *str1, void *str2) {

    return strcmp(((MonitoredCountry*)str1)->name,((MonitoredCountry*)str2)->name);
}