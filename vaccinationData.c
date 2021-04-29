#include <stdio.h>
#include <string.h>
#include "vaccinationData.h"
#include "record.h"
#include "country.h"
#include "genericHashTable.h"

extern genericHashTable tempStatisticsHash;

/*Compare two vaccData based on citizenID of indicating citizen*/
int vaccData_compare(void* data1, void* data2) {

    VaccData *vaccData1 = (VaccData*) data1;
    VaccData *vaccData2 = (VaccData*) data2;
    int result = strcmp(record_getCitizenID(vaccData1->record),record_getCitizenID(vaccData2->record));

    if (!result) vaccData2->dateVaccinated = vaccData1->dateVaccinated;
    return result;
}