#include <stdio.h>
#include <string.h>
#include "record.h"
#include "country.h"
#include "genericHashTable.h"

extern genericHashTable tempStatisticsHash;

/*The first three functions compare two given records by citizenID,
whether they have type Record** or Record**/

int record_compareType1(void* data1, void* data2){

    return strcmp((*(Record**)data1)->citizenID,((Record*)data2)->citizenID);
}

int record_compareType2(void* data1, void* data2){

    return strcmp(((Record*)data1)->citizenID,((Record*)data2)->citizenID);
}

int record_compareType3(void* data1, void* data2){

    return strcmp((*(Record**)data1)->citizenID,(*(Record**)data2)->citizenID);
}

/*Compare all elements of the two given records.
Return 0 if they are similar, 1 otherwise*/
int record_fullCompare(Record* record1, Record* record2) {

    if (strcmp(record1->citizenID,record2->citizenID) ||\
        strcmp(record1->firstName,record2->firstName) ||\
        strcmp(record1->lastName,record2->lastName) ||\
        record1->age != record2->age ||\
        strcmp(country_getName(record1->country),country_getName(record2->country)))

        return 1;
    else return 0;
}

void record_setCountry(Record* record, Country* country) {

    record->country = country;
    return;
}

Country* record_getCountry(Record* record) {

    return record->country;
}

char* record_getCitizenID(Record* record) {

    return record->citizenID;
}

char record_getAge(Record* record) {

    return record->age;
}

/*Print the elements of given record. Used in skipList_applyToAllNodes*/
int record_print(void* record, void* data1, void* data2, void* data3) {

    Record* currentRecord = *(Record**) record;

    printf("%s %s %s %s %d\n",currentRecord->citizenID, currentRecord->firstName, currentRecord->lastName,\
     country_getName(currentRecord->country),currentRecord->age);

    return 0;
}