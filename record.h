#ifndef RECORD_H
#define RECORD_H
#include "country.h"

typedef struct{

    char citizenID[11];
    char firstName[16];
    char lastName[16];
    char age;
    Country *country;
}Record;

/*Three functions comparing two given records by citizenID,
whether they have type Record** or Record**/
int record_compareType1(void*, void*);
int record_compareType2(void*, void*);
int record_compareType3(void*, void*);

/*Compare all elements of the two given records*/
int record_fullCompare(Record* , Record* );

/*Print given Record*/
int record_print(void* , void* , void* , void* );

/*Update tempStatisticsHash holding info for populationStatus command*/
int record_countryCount(void *, void *, void *, void *);

/*Update tempStatisticsHash holding info for popStatusByAge command*/
int record_countryCountByAge(void *, void *, void *, void *);

/*ADT set - get functions*/
void record_setCountry(Record* , Country* );
char* record_getCitizenID(Record* );
Country* record_getCountry(Record* );
char record_getAge(Record* );
char* record_getCitizenName(Record* );
char* record_getCitizenSurname(Record* );
char* record_getCitizenID(Record* );

#endif