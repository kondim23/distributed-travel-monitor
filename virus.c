#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "virus.h"
#include "record.h"
#include "skipList.h"
#include "bloomFilter.h"
#include "vaccinationData.h"
#include "utils.h"
#include "Monitor.h"

extern int fdes[2];
extern unsigned int buffer_size;
enum{READ,WRITE};

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
    unsigned int message_size;
    char tempString[4];

    /*If noone has been vaccinated on this virus*/
    if (virusPtr->vaccinated_persons == NULL) {

        message_size=3;
        strcpy(tempString,"NO");

	    write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
	    write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempString );

    }
    else {

        /*If user has been vaccinated on this virus*/
        if (!skipList_searchValue(virusPtr->vaccinated_persons, currentVaccData, &vaccData_compare)) {

            message_size=4;
            strcpy(tempString,"YES");

	        write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
            write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempString );

            /*write date*/
            message_size=sizeof(time_t);
	        write_to_pipe(message_size , buffer_size , fdes[WRITE] , &(currentVaccData->dateVaccinated) );


        }
        else {

            message_size=3;
            strcpy(tempString,"NO");

	        write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
	        write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempString );

        }
    }
    return;
}

/*Search given vaccination Data in virus data and print the result*/
void virus_searchRecordInVaccinatedType2(void *dataToCompare, void *currentData) {

    Virus *virusPtr = (Virus*) dataToCompare;
    VaccData *currentVaccData = (VaccData*) currentData;
    char date[11], boolReq;
    unsigned int message_size;
    char tempString[4];

    message_size=strlen(virusPtr->name)+1;

    /*write virus name*/
	write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
	write_to_pipe(message_size , buffer_size , fdes[WRITE] , virusPtr->name );


    /*If noone has been vaccinated on this virus*/
    if (virusPtr->vaccinated_persons == NULL) {

        boolReq=1;
	    write_to_pipe(sizeof(char) , buffer_size , fdes[WRITE] , &boolReq );

    }

    else {

        /*If user has been vaccinated on this virus*/
        if (!skipList_searchValue(virusPtr->vaccinated_persons, currentVaccData, &vaccData_compare)) {
            
            // strftime(date, 11, "%d-%m-%Y",localtime(&(currentVaccData->dateVaccinated)));

            /*write YES date*/
            boolReq=0;
	        write_to_pipe(sizeof(char) , buffer_size , fdes[WRITE] , &boolReq );
            
            message_size=sizeof(time_t);
	        write_to_pipe(message_size , buffer_size , fdes[WRITE] , &(currentVaccData->dateVaccinated) );

        }
        else {


            /*write NO*/
            boolReq=1;
	        write_to_pipe(sizeof(char) , buffer_size , fdes[WRITE] , &boolReq );

        }
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