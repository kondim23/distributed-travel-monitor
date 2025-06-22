#ifndef COUNTRY_H
#define COUNTRY_H

typedef struct{

    char name[16];
}Country;

typedef struct{

    char name[16];
    unsigned short int monitorNum;
}MonitoredCountry;

typedef struct{

    char name[100];
    void *fileSkipList;
}CountryFolder;

/*Return name of country*/
char* country_getName(Country *);

/*Compare two countries based on their name*/
int country_compare(void* , void* );

int countryFolders_compare(void* , void* );

int mystrcmp(void *, void* );

int monitoredCountry_compare(void *str1, void *str2);

int monitoredCountry_compareNonCap(void *, void *);

#endif