#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "genericHashTable.h"
#include "record.h"
#include "country.h"
#include "virus.h"
#include "vaccinationData.h"
#include "Monitor.h"
#include "skipList.h"
#include "bloomFilter.h"
#include "utils.h"
#include "pipe.h"


genericHashTable tempStatisticsHash, virusHash, recordsHash, countriesHash;
skipList countryFolders;
CountryFolder currentCountryFolder, *cFolder;
Record currentRecord, *recordPtr;
Country currentCountry, *countryPtr;
Virus currentVirus, *virusPtr;
VaccData currentVaccData;
DIR 	*dir_ptr;
struct 	dirent *direntp;
void *message;
FILE *citizenRecordsFile;
struct tm tempTime={0};
time_t date1, date2;
char *subFileName, *lineInput, bufferLine[200], *token, date[11], newRecords=0, countryName[20];
size_t fileBufferSize=512;
unsigned int buffer_size=10, bloomSize, acceptedReq=0, rejectedReq=0, message_size;
int fdes[2];
enum{READ,WRITE};

int main(int argc, char *argv[]) {

	char *subFileName, tempString[13], boolReq, *tempStr;
	int nwrite;

	static struct sigaction newRecordsAction;

	newRecordsAction.sa_handler=changeStatus_newRecords;
    sigfillset(&(newRecordsAction.sa_mask));

	sigaction(SIGUSR1, &newRecordsAction, NULL);


    /*Check Arguments*/

    if (argc!=2) { printf("Error in Monitor arguments\n"); exit(1); }

	if ( (fdes[READ]=open(argv[1], O_RDWR)) < 0)
		{ perror("fife open error"); exit(1); }
    if ( (fdes[WRITE]=open(argv[0], O_WRONLY| O_NONBLOCK)) < 0)
		{ perror("fife open error"); exit(1); }


 	/*Initializing hashes for country, virus and records*/
    virusHash = hash_Initialize();
    recordsHash = hash_Initialize();
    countriesHash = hash_Initialize();
	countryFolders = skipList_initializeSkipList();

	/*Get buffer size and bloom size from pipe*/

	// printf("old %d\n",buffer_size);
	// printf("old %d\n",bloomSize);
	read_from_pipe(sizeof(unsigned int),sizeof(unsigned int),fdes[READ],&buffer_size);
	read_from_pipe(sizeof(unsigned int),buffer_size,fdes[READ],&bloomSize);
	// printf("new %d\n",buffer_size);
	// printf("new %d\n",bloomSize);

    /*Get Countries Directory from pipe*/

	lineInput = malloc(sizeof(char)*fileBufferSize);

	read_from_pipe(sizeof(message_size),buffer_size,fdes[READ],&message_size);
	// printf("Message Received: %d\n", message_size);
	message = malloc(message_size);
	read_from_pipe(message_size,buffer_size,fdes[READ],message);
	printf("Monitor Received: %s\n", (char*)message);

	while (strcmp(message,"_COUNTRIES_END")) {

		strcpy(currentCountryFolder.name,(char*)message);
		currentCountryFolder.fileSkipList = skipList_initializeSkipList();
		skipList_insertValue(countryFolders,&currentCountryFolder,sizeof(CountryFolder),&countryFolders_compare);
		cFolder=skipList_searchReturnValue(countryFolders, &currentCountryFolder,&countryFolders_compare);
		if (cFolder==NULL) printf("pou sai %d\n",message_size);
		updateSystem();

		// if ((dir_ptr = opendir((char*)message)) == NULL)

		// 	fprintf(stderr, "cannot open %s \n",(char*)message);

		// else {

		// 	getSubDirName((char*)message,countryName);

		// 	while ((direntp = readdir(dir_ptr)) != NULL) {

		// 		if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) continue;

		// 		skipList_insertValue(cFolder->fileSkipList,direntp->d_name,strlen(direntp->d_name)+1,&mystrcmp);

		// 		subFileName = (char*) malloc(strlen((char*)message)+strlen(direntp->d_name)+2);
		// 		strcpy(subFileName,(char*)message);
		// 		strcat(subFileName,"/");
		// 		strcat(subFileName,direntp->d_name);

		// 		// printf("%s\n",subFileName);

		// 		citizenRecordsFile = fopen(subFileName,"r");
		// 		if (!citizenRecordsFile) {
		// 			printf("Error! Could not open %s\n",direntp->d_name);
		// 			exit(1);
		// 		}
		// 		while(getline(&lineInput, &fileBufferSize, citizenRecordsFile)!=-1) {

		// 			/*Storing record for print if error occurs*/
        // 			strcpy(bufferLine,lineInput);

		// 			/*Collecting arguments and inserting record to system*/
		// 			if (getRecordArguments(lineInput,countryName,citizenRecordsFile)) continue;
		// 			if (insertRecordToSystem(1,citizenRecordsFile) == -1) return 1;
		// 		}

		// 		free(subFileName);
		// 	}
		// 	closedir(dir_ptr);
		// }

		free(message);

		// virus_initialize(&currentVirus,"covid-19");
    	// virusPtr = (Virus*) hash_searchValue(virusHash, currentVirus.name, &currentVirus, 0, &virus_compare);
		// bloomFilter_search(virusPtr->bloomFilter,bloomSize,"1");
		// bloomFilter_search(virusPtr->bloomFilter,bloomSize,"2");
		// bloomFilter_search(virusPtr->bloomFilter,bloomSize,"3");
		// bloomFilter_search(virusPtr->bloomFilter,bloomSize,"15");

		read_from_pipe(sizeof(message_size),buffer_size,fdes[READ],&message_size);
		// printf("Message Received: %d\n", message_size);
		message = malloc(message_size);
		read_from_pipe(message_size,buffer_size,fdes[READ],message);
		printf("Monitor Received: %s\n", (char*)message);
	}
	free(message);

	sendAllBlooms();

	// /*Send bloom filters to travel Monitor*/
	// hash_applyToAllNodes(virusHash,NULL,&sendBloomThroughPipe);

	// /*Indicate the end of bloom filters*/
	// message_size=11;
    // strcpy(tempString,"_BLOOM_END");

	// write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
	// write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempString );




	/*Receive Command from Travel Monitor*/
	do{
		if (newRecords==1) {updateSystem(); sendAllBlooms();}
		read_from_pipe(sizeof(message_size),buffer_size,fdes[READ],&message_size);
	}while(newRecords==1);
	// printf("Message Received: %d\n", message_size);
	message = malloc(message_size);
	read_from_pipe(message_size,buffer_size,fdes[READ],message);
	printf("Monitor Received 1: %s\n", (char*)message);

	while(1) {
		if (!strcmp((char*)message,"_TRAVEL_REQ")) {

			read_from_pipe(sizeof(message_size),buffer_size,fdes[READ],&message_size);
			// printf("Message Received: %d\n", message_size);
			read_from_pipe(message_size,buffer_size,fdes[READ],currentRecord.citizenID);
			printf("Monitor Received 2: %s\n", currentRecord.citizenID);

			currentVaccData.record = &currentRecord;
			currentVaccData.dateVaccinated = 0;

			read_from_pipe(sizeof(message_size),buffer_size,fdes[READ],&message_size);
			// printf("Message Received: %d\n", message_size);
			message = malloc(message_size);
			read_from_pipe(message_size,buffer_size,fdes[READ],message);
			printf("Monitor Received 3: %s\n", (char*)message);

			virus_initialize(&currentVirus,(char*)message);
			free(message);

			virusPtr = (Virus*) hash_searchValue(virusHash, currentVirus.name, &currentVirus, 0, &virus_compare);
			virus_searchRecordInVaccinatedType1(virusPtr, &currentVaccData);
			
			read_from_pipe(sizeof(char),buffer_size,fdes[READ],&boolReq);
			printf("Monitor Received: %d\n", boolReq);

			if (boolReq==0) acceptedReq++;
			else rejectedReq++;
		}
		else if (!strcmp((char*)message,"_VACSTAT_REQ")) {

			read_from_pipe(sizeof(message_size),buffer_size,fdes[READ],&message_size);
			// printf("Message Received: %d\n", message_size);
			read_from_pipe(message_size,buffer_size,fdes[READ],currentRecord.citizenID);
			// printf("Monitor Received 2: %s\n", currentRecord.citizenID);

			currentVaccData.record = &currentRecord;
			currentVaccData.dateVaccinated = 0;

    		recordPtr = (Record*) hash_searchValue(recordsHash, currentRecord.citizenID, &currentRecord, 0, &record_compareType2);

			if (recordPtr!=NULL) {

				tempStr = record_getCitizenName(recordPtr);
				message_size=strlen(tempStr)+1;

				write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
				write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempStr );


				tempStr = record_getCitizenSurname(recordPtr);
				message_size=strlen(tempStr)+1;

				write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
				write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempStr );


				tempStr = country_getName(record_getCountry(recordPtr));
				message_size=strlen(tempStr)+1;

				write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
				write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempStr );


				boolReq = record_getAge(recordPtr);
				write_to_pipe(sizeof(char) , buffer_size , fdes[WRITE] , &boolReq );


            	hash_applyToAllNodes(virusHash, &currentVaccData, &virus_searchRecordInVaccinatedType2);
			}

			message_size=13;
			strcpy(tempString,"_VACSTAT_END");

			write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
			write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempString );


		}

		do{
			if (newRecords==1) {updateSystem(); sendAllBlooms();}
			read_from_pipe(sizeof(message_size),buffer_size,fdes[READ],&message_size);
		}while(newRecords==1);
		// printf("Message Received: %d\n", message_size);
		message = malloc(message_size);
		read_from_pipe(message_size,buffer_size,fdes[READ],message);
		// printf("Monitor Received 1: %s\n", (char*)message);
	}

	free(lineInput);
}

void changeStatus_newRecords(int signo) {

	newRecords=1;
	return;
}

void sendBloomThroughPipe(void *data1, void *data2) {

	Virus *virusptr = (Virus*) data1;
	int nwrite;
	unsigned int message_size=strlen(virusptr->name)+1;

	write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
	write_to_pipe(message_size , buffer_size , fdes[WRITE] , virusptr->name );
	write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &bloomSize );
	write_to_pipe(bloomSize , buffer_size , fdes[WRITE] , virusptr->bloomFilter );

	return;
}

/*receive record from input*/
int getRecordArguments(char* lineInput, char *countryName, FILE *input){

    char* token;

    /*receive full input of record and check for errors*/
    if ((token = strtok(lineInput," \t\n")) == NULL) return wrongFormat_record(input);
    strcpy(currentRecord.citizenID,token);

    if ((token = strtok(NULL," \t\n")) == NULL) return wrongFormat_record(input);
    strcpy(currentRecord.firstName,token);
    if (argumentsCheck_letters(currentRecord.firstName)) return wrongFormat_record(input);
    capitalize(currentRecord.firstName);

    if ((token = strtok(NULL," \t\n")) == NULL) return wrongFormat_record(input);
    strcpy(currentRecord.lastName,token);
    if (argumentsCheck_letters(currentRecord.lastName)) return wrongFormat_record(input);
    capitalize(currentRecord.lastName);

    strcpy(currentCountry.name,countryName);
    if (argumentsCheck_letters(currentCountry.name)) return wrongFormat_record(input);
    capitalize(currentCountry.name);
    currentRecord.country = &currentCountry;

    if ((token = strtok(NULL," \t\n")) == NULL) return wrongFormat_record(input);
    currentRecord.age = atoi(token);
    if (currentRecord.age < 1 || currentRecord.age > 120) return wrongFormat_record(input);

    if ((token = strtok(NULL," \t\n")) == NULL) return wrongFormat_record(input);
    if (argumentsCheck_lettersNumbersDash(token)) return wrongFormat_record(input);
    virus_initialize(&currentVirus,token);


	if ((token = strtok(NULL," \t\n")) == NULL) return wrongFormat_record(input);
	if (!strcmp(token,"YES")) {

		if ((token = strtok(NULL," \t\n")) == NULL) return wrongFormat_record(input);

		/*get date*/
		sscanf(token, "%2d-%2d-%4d",&tempTime.tm_mday,&tempTime.tm_mon,&tempTime.tm_year);
		tempTime.tm_mon--;
		tempTime.tm_year -= 1900;
		currentVaccData.record = &currentRecord;
		currentVaccData.dateVaccinated = mktime(&tempTime);
	}
	else if (!strcmp(token,"NO")) {
		
		if ((token = strtok(NULL," \t\n")) != NULL) return wrongFormat_record(input);
		currentVaccData.record = &currentRecord;
		currentVaccData.dateVaccinated = 0;
	}
	else return wrongFormat_record(input);

    return 0;
}

/*checks properly for errors of record and insert to system*/
/*acceptChange (0-YES 1-NO) indicates whether we received a record from the input file or the stdin*/
int insertRecordToSystem(char acceptChange, FILE *input) {

    /*Search and if not exist insert virus to system virushash*/
    virusPtr = (Virus*) hash_searchValue(virusHash, currentVirus.name, &currentVirus, sizeof(Virus), &virus_compare);

    /*Search for record in records hash*/
    recordPtr = (Record*) hash_searchValue(recordsHash, currentRecord.citizenID, &currentRecord, 0, &record_compareType2);
    
    /*If record with citizenID found and inconsistent, dump record*/
    if (recordPtr != NULL && record_fullCompare(&currentRecord, recordPtr)) {

        printf("ERROR IN RECORD %s",bufferLine);
        return 1;
    }
    
    /*If record exists in vaccinated_persons*/
    if (!skipList_searchValue(virusPtr->vaccinated_persons, &currentVaccData, &vaccData_compare)) {
        
        if (!acceptChange) {
            strftime(date, 11, "%d-%m-%Y",localtime(&(currentVaccData.dateVaccinated)));
            printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %s\n",record_getCitizenID(currentVaccData.record),date);
        }
        else printf("ERROR IN RECORD %s",bufferLine);
        return 1;
    }
    /*If record exists in not_vaccinated_persons*/
    else if (!skipList_searchValue(virusPtr->not_vaccinated_persons, &currentRecord, &record_compareType1)) {

        /*If we accept a change to a record's vaccination state from No to YES remove from not_vaccinated_persons*/
        if (!acceptChange && currentVaccData.dateVaccinated>0) 
            skipList_removeValue(virusPtr->not_vaccinated_persons, &currentRecord, &record_compareType1);
        else {

            if (acceptChange) printf("ERROR IN RECORD %s",bufferLine);
            return 2;
        }
    }
    
    /*Search and if not exist insert country to system countryhash*/
    countryPtr = (Country*) hash_searchValue(countriesHash, currentCountry.name, &currentCountry, sizeof(Country), &country_compare);
    currentRecord.country = countryPtr;

    /*Insert record in records hash*/
    recordPtr = (Record*) hash_searchValue(recordsHash, currentRecord.citizenID, &currentRecord, sizeof(Record), &record_compareType2);

    /*If record refered as NO vaccinated*/
    if (currentVaccData.dateVaccinated == 0) {

        /*Insert in not_vaccinated_persons*/
        if (virusPtr->not_vaccinated_persons == NULL) 
            virusPtr->not_vaccinated_persons = skipList_initializeSkipList();
        if (skipList_insertValue(virusPtr->not_vaccinated_persons, &recordPtr, sizeof(Record*), &record_compareType3) < 0) return -1;
    }
    /*If record refered as YES date vaccinated*/
    else {

        currentVaccData.record = recordPtr;

        /*Insert in vaccinated_persons*/
        if (virusPtr->vaccinated_persons == NULL) 
            virusPtr->vaccinated_persons = skipList_initializeSkipList();
        if (skipList_insertValue(virusPtr->vaccinated_persons, &currentVaccData, sizeof(VaccData), &vaccData_compare) < 0) return -1;

        /*Insert in bloomfitler*/
        if (virusPtr->bloomFilter == NULL) virusPtr->bloomFilter = bloomFilter_create(bloomSize);
        bloomFilter_insert(virusPtr->bloomFilter, bloomSize, recordPtr->citizenID);
    }

    return 0;
}

/*Error message for wrong record format and get new command*/
unsigned int wrongFormat_record(FILE *input) {

    printf("ERROR IN RECORD %s",bufferLine);
    getline(&lineInput, &fileBufferSize, input);
    strcpy(bufferLine,lineInput);
    token = strtok(lineInput, " \t\n");
    return 1;
}

void getSubDirName(char* path, char* subDirName) {

	int i=strlen(path)-1;
	while (i>=0 && path[i]!='/') i--;
	strcpy(subDirName,path+i+1);
	return;
}

int updateSystem() {


	if (newRecords) {

		message = malloc(message_size);
		read_from_pipe(message_size,buffer_size,fdes[READ],message);
		printf("Monitor Received: %s\n", (char*)message);
		newRecords=0;
	}

	if ((dir_ptr = opendir((char*)message)) == NULL)

		fprintf(stderr, "cannot open %s \n",(char*)message);

	else {

		getSubDirName((char*)message,countryName);

		while ((direntp = readdir(dir_ptr)) != NULL) {

			if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) continue;
			if (!skipList_searchValue(cFolder->fileSkipList,direntp->d_name,&mystrcmp)) continue;
			skipList_insertValue(cFolder->fileSkipList,direntp->d_name,strlen(direntp->d_name)+1,&mystrcmp);
			subFileName = (char*) malloc(strlen((char*)message)+strlen(direntp->d_name)+2);
			strcpy(subFileName,(char*)message);
			strcat(subFileName,"/");
			strcat(subFileName,direntp->d_name);


			citizenRecordsFile = fopen(subFileName,"r");
			if (!citizenRecordsFile) {
				printf("Error! Could not open %s\n",direntp->d_name);
				exit(1);
			}
			while(getline(&lineInput, &fileBufferSize, citizenRecordsFile)!=-1) {

				/*Storing record for print if error occurs*/
				strcpy(bufferLine,lineInput);

				/*Collecting arguments and inserting record to system*/
				if (getRecordArguments(lineInput,countryName,citizenRecordsFile)) continue;
				if (insertRecordToSystem(1,citizenRecordsFile) == -1) return 1;
			}

			free(subFileName);
		}
		closedir(dir_ptr);
	}

	return 0;
}

void sendAllBlooms(){

	unsigned int message_size;
	char tempString[11];

	message_size=1;

	/*Send bloom filters to travel Monitor*/
	hash_applyToAllNodes(virusHash,NULL,&sendBloomThroughPipe);

	/*Indicate the end of bloom filters*/
	message_size=11;
    strcpy(tempString,"_BLOOM_END");

	write_to_pipe(sizeof(unsigned int) , buffer_size , fdes[WRITE] , &message_size );
	write_to_pipe(message_size , buffer_size , fdes[WRITE] , tempString );

	return;
}