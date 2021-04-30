#include <stdio.h>
#include <string.h>
#include "virus.h"
#include "record.h"
#include "skipList.h"
#include "bloomFilter.h"
#include "vaccinationData.h"
#include "utils.h"


/*Compare two given viruses based on their name*/
int virus_compare(void* data1, void* data2){

    return strcmp( ((Virus*) data1)->name , ((Virus*) data2)->name );
}

/*Search given vaccination Data in virus data and print the result*/
/*Used in vaccineStatus with given virus*/
void virus_searchRecordInVaccinatedType1(void *dataToCompare, void *currentData) {

    Virus *virusPtr = (Virus*) dataToCompare;
    VaccData *currentVaccData = (VaccData*) currentData;
    char date[11];

    /*If noone has been vaccinated on this virus*/
    if (virusPtr->vaccinated_persons == NULL) printf("NOT VACCINATED\n");

    else {

        /*If user has been vaccinated on this virus*/
        if (!skipList_searchValue(virusPtr->vaccinated_persons, currentVaccData, &vaccData_compare)) {

            strftime(date, 11, "%d-%m-%Y",localtime(&(currentVaccData->dateVaccinated)));
            printf("VACCINATED ON %s\n",date);
        }
        else printf("NOT VACCINATED\n");
    }
    return;
}

/*Search given vaccination Data in virus data and print the result*/
/*Used in vaccineStatus without given virus*/
void virus_searchRecordInVaccinatedType2(void *dataToCompare, void *currentData) {

    Virus *virusPtr = (Virus*) dataToCompare;
    VaccData *currentVaccData = (VaccData*) currentData;
    char date[11];

    printf("%s ",virusPtr->name);

    /*If noone has been vaccinated on this virus*/
    if (virusPtr->vaccinated_persons == NULL) printf("NO\n");

    else {

        /*If user has been vaccinated on this virus*/
        if (!skipList_searchValue(virusPtr->vaccinated_persons, currentVaccData, &vaccData_compare)) {
            
            strftime(date, 11, "%d-%m-%Y",localtime(&(currentVaccData->dateVaccinated)));
            printf("YES %s\n",date);
        }
        else printf("NO\n");
    }
    return;
}

/*Initialize virus based on name given*/
void virus_initialize(Virus* virusPtr, char* name) {

    strcpy(virusPtr->name,name);
    capitalize(virusPtr->name);
    virusPtr->vaccinated_persons = NULL;
    virusPtr->not_vaccinated_persons = NULL;
    virusPtr->bloomFilter = NULL;
    return;
}

/*Free all contents of virus*/
void virus_destroy(void *data, void *data1) {

    Virus *virus = (Virus*) data;

    skipList_destroy(virus->not_vaccinated_persons);
    skipList_destroy(virus->vaccinated_persons);
    bloomFilter_delete(virus->bloomFilter);
    return;
}