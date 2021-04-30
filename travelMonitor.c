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

void read_from_pipe(unsigned int , unsigned int , int , void* );
int writeSubdirToPipe(void *, void *, void *, void *);
int mystrcmp(void *, void *);

Virus currentVirus, *virusptr;
struct tm tempTime={0};
time_t date1, date2;
char *subdirectory, *inputDir, *lineInput, bufferLine[200], *token, date[11];
size_t fileBufferSize=512;
struct 	dirent *direntp;
unsigned int numMonitors, message_size;
int nwrite;
enum{READ,WRITE};

int main(int argc, char *argv[]) {

    unsigned int bufferSize, sizeOfBloom;
    char fifoName[2][14], tempString[15];
    int pid, i;
    DIR 	*dir_ptr;
    void *message;
    skipList countriesSkipList;


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

        if ((nwrite=write(fd[i][WRITE], &bufferSize, sizeof(unsigned int))) == -1) {
            perror("Error in Writing"); exit(2);
        }
        // printf("%d\n",nwrite);

        if ((nwrite=write(fd[i][WRITE], &sizeOfBloom, sizeof(unsigned int))) == -1) {
            perror("Error in Writing"); exit(2);
        }
        // printf("%d\n",nwrite);
    }

    /*Sharing countries subfolders to Monitors*/

    if ((dir_ptr = opendir(inputDir)) == NULL)

        fprintf(stderr, "cannot open %s \n",inputDir);

    else {

        /*Inserting subDirs to countriesSkipList, which results to receiving countries alphabetically*/
        countriesSkipList = skipList_initializeSkipList();

        while ((direntp = readdir(dir_ptr)) != NULL) {

            if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) continue;
            skipList_insertValue(countriesSkipList,direntp->d_name,strlen(direntp->d_name)+1,&mystrcmp);
        }

        skipList_applyToAll(countriesSkipList,fd,NULL,NULL,&writeSubdirToPipe);
        closedir(dir_ptr);
    }
    free(inputDir);



    /*Send _COUNTRIES_END to Monitors*/

    message_size=15;
    strcpy(tempString,"_COUNTRIES_END");
    for (int i=0 ; i<numMonitors ; i++) {
    
        if ((nwrite=write(fd[i][WRITE], &message_size, sizeof(unsigned int))) == -1) {
            perror("Error in Writing"); exit(2);
        }
        // printf("%d\n",nwrite);

        if ((nwrite=write(fd[i][WRITE], tempString, message_size)) == -1) {
            perror("Error in Writing"); exit(2);
        }
        // printf("%d\n",nwrite);
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

        }
        else if (!strcmp(token,"/travelStats")) {

        }
        else if (!strcmp(token,"/addVaccinationRecords")) {

        }
        else if (!strcmp(token,"/searchVaccinationStatus")) {

        }
        else printf("Please type a valid command.\n");

        getline(&lineInput, &fileBufferSize, stdin);
        strcpy(bufferLine,lineInput);
        token = strtok(lineInput, " \t\n");
    }

    free(lineInput);
    skipList_destroy(countriesSkipList);
    for (i=0 ; i<numMonitors ; i++) hash_destroy(bloomHashes[i]);
    sleep(5);
}

int writeSubdirToPipe(void *data1, void *fd, void *data3, void *data4) {

    static int i=0;

    subdirectory = (char*) malloc(strlen(inputDir)+strlen((char*)data1)+2);
    strcpy(subdirectory,inputDir);
    strcat(subdirectory,"/");
    strcat(subdirectory,(char*)data1);
    printf("%d %s %s %ld\n",i,(char*)data1, subdirectory, strlen(subdirectory));

    message_size = strlen(subdirectory)+1;

    if ((nwrite=write(((int(*)[2])fd)[i][WRITE], &message_size, sizeof(unsigned int))) == -1) {
        perror("Error in Writing"); exit(2);
    }
    // printf("%d\n",nwrite);

    if ((nwrite=write(((int(*)[2])fd)[i][WRITE], subdirectory, message_size)) == -1) {
        perror("Error in Writing"); exit(2);
    }
    // printf("%d\n",nwrite);

    free(subdirectory);
    i = (++i) % numMonitors;
}

int mystrcmp(void *str1, void *str2) {

    return strcmp((char*)str1,(char*)str2);
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