#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "genericHashTable.h"
#include "virus.h"
#include "bloomFilter.h"
#include "skipList.h"
#include "country.h"

void read_from_pipe(unsigned int , unsigned int , int , void* );
void write_to_pipe(unsigned int , unsigned int , int , void* );
int writeSubdirToPipe(void *, void *, void *, void *);
int mystrcmp(void *, void *);
unsigned int wrongFormat_command();

Virus currentVirus, *virusptr;
struct tm tempTime={0};
time_t date1, date2, dateTemp;
char *subdirectory, *inputDir, *lineInput, bufferLine[200], *token, date[11];
size_t fileBufferSize=512;
struct 	dirent *direntp;
unsigned int bufferSize, numMonitors, message_size,buffer_size, acceptedReq=0, rejectedReq=0;
int nwrite;
int fdes[2];
enum{READ,WRITE};

int main(int argc, char *argv[]) {

    unsigned int sizeOfBloom;
    char fifoName[2][14], tempString[13], citizenID[4], virusName[20], countryFrom[20], countryTo[20], boolReq;
    int pid, i;
    DIR 	*dir_ptr;
    void *message;
    skipList countriesSkipList;
    MonitoredCountry m_country, *MCountryPtr;


    /*Checking User's arguments*/

    if (argc!=9) {

        printf("Error! Wrong input!\n");
        return 1;
    }

    for (int i=1 ; i<9 ; i+=2) {

        if(!strcmp(argv[i],"-m")) numMonitors = atoi(argv[i+1]);
        else if(!strcmp(argv[i],"-b")) bufferSize = atoi(argv[i+1]);
        else if(!strcmp(argv[i],"-s")) sizeOfBloom = atoi(argv[i+1]);
        else if(!strcmp(argv[i],"-i")) {

            inputDir = (char*) malloc(strlen(argv[i+1])*sizeof(char)+1);
            strcpy(inputDir,argv[i+1]);
        }
    }

    /*Storing file descriptors*/
    int fd[numMonitors][2];

    /*bloomHashes hold numMonitors hashes of blooms (a bloom filter per virus)*/
    genericHashTable bloomHashes[numMonitors];
    for (i=0 ; i<numMonitors ; i++) bloomHashes[i]=NULL;

    /*Creating Named Pipes and Monitor Processes*/

    for (int i=0 ; i<numMonitors ; i++) {

        sprintf(fifoName[READ], "mon%d_to_tm",i);
        if ( mkfifo(fifoName[READ], 0666) == -1 ){
		    if ( errno!=EEXIST ) { perror("Cant create named pipe!"); exit(6); };
		}
        if ( (fd[i][READ]=open(fifoName[READ], O_RDWR)) < 0){
		    perror("Cant open named pipe!"); exit(3);	
		}

        sprintf(fifoName[WRITE], "tm_to_mon%d",i);
        if ( mkfifo(fifoName[WRITE], 0666) == -1 ){
		    if ( errno!=EEXIST ) { perror("Cant create named pipe!"); exit(6); };
		}
        if ( (fd[i][WRITE]=open(fifoName[WRITE], O_RDWR)) < 0){
		    perror("Cant open named pipe!"); exit(3);	
		}

        pid = fork();
        switch(pid) {

            case -1:
                perror("fork failed\n");
                exit(1);

            case 0:
                execlp("./Monitor",fifoName[READ],fifoName[WRITE],NULL);
	    }

        write_to_pipe(sizeof(unsigned int) , bufferSize , fd[i][WRITE] , &bufferSize );
        write_to_pipe(sizeof(unsigned int) , bufferSize , fd[i][WRITE] , &sizeOfBloom );

    }

    /*Sharing countries subfolders to Monitors*/

    if ((dir_ptr = opendir(inputDir)) == NULL)

        fprintf(stderr, "cannot open %s \n",inputDir);

    else {

        /*Inserting subDirs to countriesSkipList, which results to receiving countries alphabetically*/
        countriesSkipList = skipList_initializeSkipList();
        m_country.monitorNum=0;

        while ((direntp = readdir(dir_ptr)) != NULL) {

            if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) continue;
            strcpy(m_country.name,direntp->d_name);
            skipList_insertValue(countriesSkipList,&m_country,sizeof(MonitoredCountry),&monitoredCountry_compare);
        }

        skipList_applyToAll(countriesSkipList,fd,NULL,NULL,&writeSubdirToPipe);
        closedir(dir_ptr);
    }
    free(inputDir);



    /*Send _COUNTRIES_END to Monitors*/

    message_size=15;
    strcpy(tempString,"_COUNTRIES_END");
    for (int i=0 ; i<numMonitors ; i++) {

        write_to_pipe(sizeof(unsigned int) , bufferSize , fd[i][WRITE] , &message_size );
        write_to_pipe(message_size , bufferSize , fd[i][WRITE] , tempString );

    }


    /*Get Bloom filters*/

    for (int i=0 ; i<numMonitors ; i++) {

        read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
		message = malloc(message_size);
		read_from_pipe(message_size,bufferSize,fd[i][READ],message);
		printf("Travel Received: %s\n", (char*)message);

	    while (strcmp(message,"_BLOOM_END")) {

            /*Insert in system*/

            virus_initialize(&currentVirus,(char*)message);
            free(message);

            if (bloomHashes[i]==NULL) bloomHashes[i]=hash_Initialize();
            virusptr = (Virus*) hash_searchValue(bloomHashes[i],currentVirus.name,&currentVirus,sizeof(Virus),&virus_compare);
            virusptr->bloomFilter = bloomFilter_create(sizeOfBloom);

            read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
            message = malloc(message_size);
            read_from_pipe(message_size,bufferSize,fd[i][READ],virusptr->bloomFilter);
            printf("Travel Received: %s\n", (char*)virusptr->bloomFilter);

            free(message);

            read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
            message = malloc(message_size);
            read_from_pipe(message_size,bufferSize,fd[i][READ],message);
		    printf("Travel Received: %s\n", (char*)message);
        }
        free(message);
    }

    /*Getting user's input*/

	lineInput = malloc(sizeof(char)*fileBufferSize);

    getline(&lineInput, &fileBufferSize, stdin);
    strcpy(bufferLine,lineInput);
    token = strtok(lineInput, " \t\n");

    while (strcmp("/exit",token)) {

        if (!strcmp(token,"/travelRequest")) {

            /*Get Command Arguments*/

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            strcpy(citizenID,token);

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            sscanf(token, "%2d-%2d-%4d",&tempTime.tm_mday,&tempTime.tm_mon,&tempTime.tm_year);
            tempTime.tm_mon--;
            tempTime.tm_year -= 1900;
            date1 = mktime(&tempTime);

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            strcpy(countryFrom,token);

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            strcpy(countryTo,token);

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            virus_initialize(&currentVirus,token);

            if ((token = strtok(NULL," \t\n")) != NULL)  {wrongFormat_command(); continue;}

            /*Search in Bloom Filter*/

            strcpy(m_country.name,countryFrom);

            MCountryPtr=skipList_searchReturnValue(countriesSkipList,&m_country,&monitoredCountry_compare);
            if (MCountryPtr==NULL) {

                printf("REQUEST REJECTED – YOU ARE NOT VACCINATED 1\n");
                getline(&lineInput, &fileBufferSize, stdin);
                strcpy(bufferLine,lineInput);
                token = strtok(lineInput, " \t\n");
                continue;
            }

            virusptr=hash_searchValue(bloomHashes[MCountryPtr->monitorNum],currentVirus.name,&currentVirus,0,virus_compare);
            if (bloomFilter_search(virusptr->bloomFilter,sizeOfBloom,citizenID)){

                printf("REQUEST REJECTED – YOU ARE NOT VACCINATED 2\n");
                getline(&lineInput, &fileBufferSize, stdin);
                strcpy(bufferLine,lineInput);
                token = strtok(lineInput, " \t\n");
                continue;
            }

            message_size = 12;
            strcpy(tempString,"_TRAVEL_REQ");

            write_to_pipe(sizeof(unsigned int) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &message_size );
            write_to_pipe(message_size , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , tempString );
            write_to_pipe(sizeof(unsigned int) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &message_size );
            write_to_pipe(message_size , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , citizenID );
            write_to_pipe(sizeof(unsigned int) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &message_size );
            write_to_pipe(message_size , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , currentVirus.name );

            read_from_pipe(sizeof(message_size),bufferSize,fd[MCountryPtr->monitorNum][READ],&message_size);
            message = malloc(message_size);
            read_from_pipe(message_size,bufferSize,fd[MCountryPtr->monitorNum][READ],message);
            printf("Travel Received: %s\n", (char*)message);

            if (!strcmp((char*)message,"NO")) {

                printf("REQUEST REJECTED – YOU ARE NOT VACCINATED 3\n");
                free(message);
                getline(&lineInput, &fileBufferSize, stdin);
                strcpy(bufferLine,lineInput);
                token = strtok(lineInput, " \t\n");
                continue;
            }
            free(message);
               
            read_from_pipe(sizeof(time_t),bufferSize,fd[MCountryPtr->monitorNum][READ],&date2);
            printf("Travel Received: %d\n", (int)date2);

            if (difftime(date1,date2)<=15552000.0){

                printf("REQUEST ACCEPTED – HAPPY TRAVELS\n");
                acceptedReq++;
                boolReq = 0;
            }
            else {
                printf("REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n");
                rejectedReq++;
                boolReq = 1;
            }
            write_to_pipe(sizeof(char) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &boolReq );

    
        }
        else if (!strcmp(token,"/travelStats")) {

        }
        else if (!strcmp(token,"/addVaccinationRecords")) {

        }
        else if (!strcmp(token,"/searchVaccinationStatus")) {

            /*Get Command Arguments*/

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            strcpy(citizenID,token);

            for (i=0 ; i<numMonitors ; i++) {

                message_size = 13;
                strcpy(tempString,"_VACSTAT_REQ");

                write_to_pipe(sizeof(unsigned int) , bufferSize , fd[i][WRITE] , &message_size );
                write_to_pipe(message_size , bufferSize , fd[i][WRITE] , tempString );


                message_size = strlen(citizenID)+1;

                write_to_pipe(sizeof(unsigned int) , bufferSize , fd[i][WRITE] , &message_size );
                write_to_pipe(message_size , bufferSize , fd[i][WRITE] , citizenID );

            }

            for (i=0 ; i<numMonitors ; i++) {

                read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                message = malloc(message_size);
                read_from_pipe(message_size,bufferSize,fd[i][READ],message);
                // printf("Travel Received: %s\n", (char*)message);

                if (strcmp((char*)message,"_VACSTAT_END")) {

                    printf("%s %s ",citizenID,(char*)message);

                    free(message);
                    read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                    message = malloc(message_size);
                    read_from_pipe(message_size,bufferSize,fd[i][READ],message);
                    // printf("Travel Received: %s\n", (char*)message);

                    printf("%s ",(char*)message);

                    free(message);
                    read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                    message = malloc(message_size);
                    read_from_pipe(message_size,bufferSize,fd[i][READ],message);
                    // printf("Travel Received: %s\n", (char*)message);

                    printf("%s\n",(char*)message);

                    read_from_pipe(sizeof(char),bufferSize,fd[i][READ],message);
                    // printf("Travel Received: %d\n", *(char*)message);

                    printf("AGE %d\n",*(char*)message);

                    free(message);
                    read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                    message = malloc(message_size);
                    read_from_pipe(message_size,bufferSize,fd[i][READ],message);
                    // printf("Travel Received: %s\n", (char*)message);

                    while (strcmp((char*)message,"_VACSTAT_END")) {

                        printf("%s ",(char*)message);

                        read_from_pipe(sizeof(char),bufferSize,fd[i][READ],&boolReq);
                        // printf("Travel Received: %d\n", boolReq);

                        if (boolReq==0) {

                            read_from_pipe(sizeof(time_t),bufferSize,fd[i][READ],&date2);
                            // printf("Travel Received: %d\n", (int)date2);

                            strftime(date, 11, "%d-%m-%Y",localtime(&(date2)));
                            printf("VACCINATED ON %s\n",date);
                        }
                        else printf("NOT YET VACCINATED\n");

                        free(message);
                        read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                        message = malloc(message_size);
                        read_from_pipe(message_size,bufferSize,fd[i][READ],message);
                        // printf("Travel Received: %s\n", (char*)message);

                    }

                }
                free(message);
            }

        }
        else printf("Please type a valid command.\n");

        getline(&lineInput, &fileBufferSize, stdin);
        strcpy(bufferLine,lineInput);
        token = strtok(lineInput, " \t\n");
    }

    free(lineInput);
    skipList_destroy(countriesSkipList);
    for (i=0 ; i<numMonitors ; i++) hash_destroy(bloomHashes[i]);
}

int writeSubdirToPipe(void *data1, void *fd, void *data3, void *data4) {

    static int i=0;
    MonitoredCountry *m_country = (MonitoredCountry*) data1;
    m_country->monitorNum = i;

    subdirectory = (char*) malloc(strlen(inputDir)+strlen(m_country->name)+2);
    strcpy(subdirectory,inputDir);
    strcat(subdirectory,"/");
    strcat(subdirectory,m_country->name);
    printf("%d %s %s %ld\n",i,m_country->name, subdirectory, strlen(subdirectory));

    message_size = strlen(subdirectory)+1;

    write_to_pipe(sizeof(unsigned int) , bufferSize , ((int(*)[2])fd)[i][WRITE] , &message_size );
    write_to_pipe(message_size , bufferSize , ((int(*)[2])fd)[i][WRITE] , subdirectory );


    free(subdirectory);
    i = (++i) % numMonitors;
}

void read_from_pipe(unsigned int message_size, unsigned int buffer_size, int fd, void* message) {


	unsigned int bytes_to_read, btr_bac, bytes_read, bytes_read_total;
	void* msgbuf = malloc(buffer_size);


	for (int i=0 ; message_size>0 ; i++) {

		bytes_to_read = message_size>buffer_size ? buffer_size : message_size;
		btr_bac = bytes_to_read;
		bytes_read_total = 0;

		do {
			if ( (bytes_read = read(fd, msgbuf+bytes_read_total, bytes_to_read)) < 0) {
				perror("problem in reading"); exit(5);
			}
			bytes_to_read -= bytes_read;
			bytes_read_total += bytes_read;
		} while (bytes_to_read>0);

		message_size -= btr_bac;
		memcpy(message+i*buffer_size,msgbuf,btr_bac);
	}
	free (msgbuf);
	return;
}

void write_to_pipe(unsigned int message_size, unsigned int buffer_size, int fd, void* message) {

	unsigned int bytes_to_write, btw_bac, bytes_written, total_written_bytes;
	void* msgbuf = malloc(buffer_size);
	
	for (int i=0 ; message_size>0 ; i++) {

		bytes_to_write = message_size>buffer_size ? buffer_size : message_size;
		btw_bac = bytes_to_write;
		total_written_bytes = 0;
		memcpy(msgbuf,message+i*buffer_size,btw_bac);

		do {
			if ( (bytes_written = write(fd, msgbuf+total_written_bytes, bytes_to_write)) < 0) {
				perror("problem in writing"); exit(5);
			}
			bytes_to_write -= bytes_written;
			total_written_bytes += bytes_written;
		} while (bytes_to_write>0);

		message_size -= btw_bac;
	}
	free (msgbuf);
	return;
}

/*Error message for wrong command format and get new command*/
unsigned int wrongFormat_command() {

    printf("ERROR\n");
    getline(&lineInput, &fileBufferSize, stdin);
    strcpy(bufferLine,lineInput);
    token = strtok(lineInput, " \t\n");
    return 1;
}