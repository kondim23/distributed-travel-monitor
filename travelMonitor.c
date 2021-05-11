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
#include <signal.h>
#include <sys/wait.h>
#include <poll.h>
#include "genericHashTable.h"
#include "virus.h"
#include "bloomFilter.h"
#include "skipList.h"
#include "country.h"
#include "pipe.h"
#include "travelMonitor.h"
#include "request.h"
#include "utils.h"

Virus currentVirus, *virusptr;
genericHashTable requestsHash;
genericHashTable *bloomHashes;
Request currentRequest, *requestPtr;
ReqCompare reqCompare;
skipList countriesSkipList;
struct pollfd *pfds;
struct tm tempTime={0};
time_t date1, date2, dateTemp;
char tempString[20], fifoName[2][14], *subdirectory, *inputDir, *lineInput=NULL, bufferLine[200], *token, date[11], running=1;
size_t fileBufferSize=512;
struct 	dirent *direntp;
unsigned int bufferSize, numMonitors, message_size,buffer_size, acceptedReq=0, rejectedReq=0;
int nwrite, nfds;
int pid, *monitor_pid;
int fdes[2];
int fd[300][2];
unsigned int sizeOfBloom, readsRemaining;
void *message;
enum{READ,WRITE};

int main(int argc, char *argv[]) {

    char citizenID[5], virusName[20], countryFrom[20], countryTo[20], boolReq;
    int i;
    DIR 	*dir_ptr;
    MonitoredCountry m_country, *MCountryPtr;

    static struct sigaction terminateAction, childFailsAction;

    /*Handling SIGINT SIGQUIT*/
	terminateAction.sa_handler=changeStatus_running;
    sigfillset(&(terminateAction.sa_mask));

	sigaction(SIGINT, &terminateAction, NULL);
	sigaction(SIGQUIT, &terminateAction, NULL);

    /*Handling SIGCHLD*/
    childFailsAction.sa_handler=recreateChild;
    sigfillset(&(childFailsAction.sa_mask));

	sigaction(SIGCHLD, &childFailsAction, NULL);


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
            if (inputDir[strlen(inputDir)-1]=='/') inputDir[strlen(inputDir)-1]='\0';
        }
    }

    /*Storing file descriptors*/
    monitor_pid = (int*) malloc(sizeof(int)*numMonitors);
    nfds = numMonitors;
    pfds = malloc(nfds*sizeof(struct pollfd));

    /*bloomHashes hold numMonitors hashes of blooms (a bloom filter per virus)*/
    bloomHashes = (genericHashTable*) malloc(sizeof(genericHashTable)*numMonitors);
    for (i=0 ; i<numMonitors ; i++) bloomHashes[i]=NULL;

    /*requestsHash holds the travel request data from user*/
    requestsHash = hash_Initialize();

    /*Creating Named Pipes and Monitor Processes*/

    for (int i=0 ; i<numMonitors ; i++) create_pipes_and_monitors(i);

    /*Sharing countries subfolders to Monitors*/

    if ((dir_ptr = opendir(inputDir)) == NULL)

        fprintf(stderr, "cannot open %s \n",inputDir);

    else {

        /*Inserting country subDirs to countriesSkipList, which results to receiving countries alphabetically*/
        countriesSkipList = skipList_initializeSkipList();
        m_country.monitorNum=0;

        while ((direntp = readdir(dir_ptr)) != NULL) {

            if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) continue;
            strcpy(m_country.name,direntp->d_name);
            skipList_insertValue(countriesSkipList,&m_country,sizeof(MonitoredCountry),&monitoredCountry_compareNonCap);
        }

        /*writing the countries subdir to every monitor*/
        skipList_applyToAll(countriesSkipList,fd,NULL,NULL,&writeSubdirToPipe);
        closedir(dir_ptr);
    }


    /*Send _COUNTRIES_END to Monitors*/

    message_size=15;
    strcpy(tempString,"_COUNTRIES_END");
    for (int i=0 ; i<numMonitors ; i++) {

        write_to_pipe(sizeof(unsigned int) , bufferSize , fd[i][WRITE] , &message_size );
        write_to_pipe(message_size , bufferSize , fd[i][WRITE] , tempString );

    }


    /*Get Bloom filters. Choosing file descriptor with poll*/


    readsRemaining = numMonitors;

    while(readsRemaining!=0) {

        if (poll(pfds,nfds,-1)==-1) exit(1);

        for (i=0 ; i<nfds ; i++) {
            if (pfds[i].revents & POLLIN) {

                receive_bloom_filters(i);
                readsRemaining--;
            }
        }
    }

    /*Getting user's input*/

	lineInput = malloc(sizeof(char)*fileBufferSize);
    char* k;

    /*Checking if value of var running chenged by SIGINT signal*/
    if (!running) terminateProgram();

    /*Get command*/
    while ((k=fgets(lineInput, fileBufferSize, stdin))==NULL || !strcmp(lineInput,"\n"))
        if (!running) terminateProgram();

    strcpy(bufferLine,lineInput);
    token = strtok(lineInput, " \t\n");

    if (!running) terminateProgram();

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
            capitalize(countryFrom);

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            strcpy(countryTo,token);
            capitalize(countryTo);

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            virus_initialize(&currentVirus,token);

            if ((token = strtok(NULL," \t\n")) != NULL)  {wrongFormat_command(); continue;}

            /*Search in Bloom Filter*/

            strcpy(m_country.name,countryFrom);

            /*Searching country in countriesskipList to get the matched monitorNum*/
            MCountryPtr=skipList_searchReturnValue(countriesSkipList,&m_country,&monitoredCountry_compare);
            if (MCountryPtr==NULL) {

                printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
                while (fgets(lineInput, fileBufferSize, stdin)==NULL || !strcmp(lineInput,"\n"))
                    if (!running) terminateProgram();
                strcpy(bufferLine,lineInput);
                token = strtok(lineInput, " \t\n");
                continue;
            }

            /*Searching the virus in the virus-hash indicating monitorNum, and then searching its bloomfitler*/
            virusptr=hash_searchValue(bloomHashes[MCountryPtr->monitorNum],currentVirus.name,&currentVirus,0,virus_compare);
            if (virusptr==NULL || bloomFilter_search(virusptr->bloomFilter,sizeOfBloom,citizenID)){

                printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
                while (fgets(lineInput, fileBufferSize, stdin)==NULL || !strcmp(lineInput,"\n"))
                    if (!running) terminateProgram();
                strcpy(bufferLine,lineInput);
                token = strtok(lineInput, " \t\n");
                continue;
            }

            /*Sending _TRAVEL_REQ , citizenID and virusName to proper monitor*/
            message_size = 12;
            strcpy(tempString,"_TRAVEL_REQ");

            write_to_pipe(sizeof(unsigned int) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &message_size );
            write_to_pipe(message_size , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , tempString );
            write_to_pipe(sizeof(unsigned int) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &message_size );
            write_to_pipe(message_size , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , citizenID );
            write_to_pipe(sizeof(unsigned int) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &message_size );
            write_to_pipe(message_size , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , currentVirus.name );

            /*The monitor answers YES followed by date or NO*/
            read_from_pipe(sizeof(message_size),bufferSize,fd[MCountryPtr->monitorNum][READ],&message_size);
            message = malloc(message_size);
            read_from_pipe(message_size,bufferSize,fd[MCountryPtr->monitorNum][READ],message);

            if (!strcmp((char*)message,"NO")) {

                printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
                free(message);
                while (fgets(lineInput, fileBufferSize, stdin)==NULL || !strcmp(lineInput,"\n"))
                    if (!running) terminateProgram();
                strcpy(bufferLine,lineInput);
                token = strtok(lineInput, " \t\n");
                continue;
            }
            free(message);
               
            /*Reading date*/
            read_from_pipe(sizeof(time_t),bufferSize,fd[MCountryPtr->monitorNum][READ],&date2);

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

            /*Writing back to pipe the result of the request*/
            write_to_pipe(sizeof(char) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &boolReq );

            strcpy(currentRequest.countryName,countryTo);
            strcpy(currentRequest.virusName,currentVirus.name);
            currentRequest.dateOfRequest = date1;
            currentRequest.boolReq = boolReq;

            /*Inserting request in requests hash*/
            hash_insertDupAllowed(requestsHash,currentRequest.virusName,&currentRequest,sizeof(Request));

        }
        else if (!strcmp(token,"/travelStats")) {

            /*Get user's arguments*/

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            strcpy(reqCompare.virusName,token);
            capitalize(reqCompare.virusName);

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            sscanf(token, "%2d-%2d-%4d",&tempTime.tm_mday,&tempTime.tm_mon,&tempTime.tm_year);
            tempTime.tm_mon--;
            tempTime.tm_year -= 1900;
            date1 = mktime(&tempTime);
            reqCompare.date1 = date1;

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            sscanf(token, "%2d-%2d-%4d",&tempTime.tm_mday,&tempTime.tm_mon,&tempTime.tm_year);
            tempTime.tm_mon--;
            tempTime.tm_year -= 1900;
            date2 = mktime(&tempTime);
            reqCompare.date2 = date2;

            if ((token = strtok(NULL," \t\n")) != NULL) {
                strcpy(reqCompare.countryName,token);
                capitalize(reqCompare.countryName); 
            }
            else strcpy(reqCompare.countryName,"");

            reqCompare.statistics.acceptedReq=0;
            reqCompare.statistics.rejectedReq=0;

            /*Calculating statistics by applying statistics_compute to all request hashnodes*/
            hash_applyToAllNodes(requestsHash,&reqCompare,&statistics_compute);
            printf("TOTAL REQUESTS %d\nACCEPTED %d\nREJECTED %d\n",reqCompare.statistics.acceptedReq+reqCompare.statistics.rejectedReq,\
            reqCompare.statistics.acceptedReq,reqCompare.statistics.rejectedReq);
        }
        else if (!strcmp(token,"/addVaccinationRecords")) {

            /*Get user's arguments*/
            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            strcpy(countryFrom,token);
            capitalize(countryFrom);

            strcpy(m_country.name,countryFrom);

            /*Return pointer to desired country*/
            MCountryPtr=skipList_searchReturnValue(countriesSkipList,&m_country,&monitoredCountry_compare);

            /*Constructing path for monitor*/
            subdirectory = (char*) malloc(strlen(inputDir)+strlen(MCountryPtr->name)+2);
            strcpy(subdirectory,inputDir);
            strcat(subdirectory,"/");
            strcat(subdirectory,MCountryPtr->name);

            /*Send SIGUSR1 signal to monitor*/
            kill(monitor_pid[MCountryPtr->monitorNum],SIGUSR1);
            message_size = strlen(subdirectory)+1;

            /*Writing to monitor the specific country path in case it manages more that 1 country*/
            write_to_pipe(sizeof(unsigned int) , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , &message_size );
            write_to_pipe(message_size , bufferSize , fd[MCountryPtr->monitorNum][WRITE] , subdirectory );


            free(subdirectory);

            hash_destroy(bloomHashes[MCountryPtr->monitorNum]);
            bloomHashes[MCountryPtr->monitorNum] = NULL;

            /*Reading new blooms from monitor*/
            read_from_pipe(sizeof(message_size),bufferSize,fd[MCountryPtr->monitorNum][READ],&message_size);
            message = malloc(message_size);
            read_from_pipe(message_size,bufferSize,fd[MCountryPtr->monitorNum][READ],message);

            while (strcmp(message,"_BLOOM_END")) {

                /*Insert in system*/

                virus_initialize(&currentVirus,(char*)message);
                free(message);

                /*Initializing blooms hash*/
                if (bloomHashes[MCountryPtr->monitorNum]==NULL) bloomHashes[MCountryPtr->monitorNum]=hash_Initialize();

                /*Inserting virus on blooms hash*/
                virusptr = (Virus*) hash_searchValue(bloomHashes[MCountryPtr->monitorNum],currentVirus.name,&currentVirus,sizeof(Virus),&virus_compare);
                virusptr->bloomFilter = bloomFilter_create(sizeOfBloom);

                /*Saving bloomfilter*/
                read_from_pipe(sizeof(message_size),bufferSize,fd[MCountryPtr->monitorNum][READ],&message_size);
                message = malloc(message_size);
                read_from_pipe(message_size,bufferSize,fd[MCountryPtr->monitorNum][READ],virusptr->bloomFilter);

                free(message);

                /*Read next bloom filter*/
                read_from_pipe(sizeof(message_size),bufferSize,fd[MCountryPtr->monitorNum][READ],&message_size);
                message = malloc(message_size);
                read_from_pipe(message_size,bufferSize,fd[MCountryPtr->monitorNum][READ],message);
            }
            free(message);

        }
        else if (!strcmp(token,"/searchVaccinationStatus")) {

            /*Get Command Arguments*/

            if ((token = strtok(NULL," \t\n")) == NULL)  {wrongFormat_command(); continue;}
            strcpy(citizenID,token);

            /*Sending to all monitors the _VACSTAT_REQ flag and the citizenID*/
            for (i=0 ; i<numMonitors ; i++) {

                message_size = 13;
                strcpy(tempString,"_VACSTAT_REQ");

                write_to_pipe(sizeof(unsigned int) , bufferSize , fd[i][WRITE] , &message_size );
                write_to_pipe(message_size , bufferSize , fd[i][WRITE] , tempString );


                message_size = strlen(citizenID)+1;

                write_to_pipe(sizeof(unsigned int) , bufferSize , fd[i][WRITE] , &message_size );
                write_to_pipe(message_size , bufferSize , fd[i][WRITE] , citizenID );

            }

            readsRemaining = numMonitors;

            /*Receiving data from monitors using poll*/
            while(readsRemaining!=0) {

                if (poll(pfds,nfds,-1)==-1) exit(1);

                for (i=0 ; i<nfds ; i++) {
                    if (pfds[i].revents & POLLIN) {

                        /*Reading first message*/
                        read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                        message = malloc(message_size);
                        read_from_pipe(message_size,bufferSize,fd[i][READ],message);

                        /*If monitor owns the user, we get and print its data*/
                        if (strcmp((char*)message,"_VACSTAT_END")) {

                            printf("%s %s ",citizenID,(char*)message);

                            /*Get name*/
                            free(message);
                            read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                            message = malloc(message_size);
                            read_from_pipe(message_size,bufferSize,fd[i][READ],message);

                            printf("%s ",(char*)message);

                            /*Get surname*/
                            free(message);
                            read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                            message = malloc(message_size);
                            read_from_pipe(message_size,bufferSize,fd[i][READ],message);

                            printf("%s\n",(char*)message);

                            /*Get age*/
                            read_from_pipe(sizeof(char),bufferSize,fd[i][READ],message);

                            printf("AGE %d\n",*(char*)message);

                            free(message);
                            read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                            message = malloc(message_size);
                            read_from_pipe(message_size,bufferSize,fd[i][READ],message);

                            /*We get data for all the viruses available to monitor*/
                            while (strcmp((char*)message,"_VACSTAT_END")) {

                                printf("%s ",(char*)message);

                                read_from_pipe(sizeof(char),bufferSize,fd[i][READ],&boolReq);

                                if (boolReq==0) {

                                    read_from_pipe(sizeof(time_t),bufferSize,fd[i][READ],&date2);

                                    strftime(date, 11, "%d-%m-%Y",localtime(&(date2)));
                                    printf("VACCINATED ON %s\n",date);
                                }
                                else printf("NOT YET VACCINATED\n");

                                free(message);
                                read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
                                message = malloc(message_size);
                                read_from_pipe(message_size,bufferSize,fd[i][READ],message);

                            }

                        }
                        free(message);
                        readsRemaining--;
                    }
                }
            }
        }
        else printf("Please type a valid command.\n");

        /*Get next command*/
        if (!running) terminateProgram();
        while (fgets(lineInput, fileBufferSize, stdin)==NULL || !strcmp(lineInput,"\n"))
            if (!running) terminateProgram();
        strcpy(bufferLine,lineInput);
        token = strtok(lineInput, " \t\n");
        if (!running) terminateProgram();
    }

    changeStatus_running(SIGINT);
    terminateProgram();
}

/*Constructing and writing the given substring to monitor*/
int writeSubdirToPipe(void *data1, void *fd, void *data3, void *data4) {

    static int i=0;
    MonitoredCountry *m_country = (MonitoredCountry*) data1;
    m_country->monitorNum = i;

    /*Constructing directory path*/
    subdirectory = (char*) malloc(strlen(inputDir)+strlen(m_country->name)+2);
    strcpy(subdirectory,inputDir);
    strcat(subdirectory,"/");
    strcat(subdirectory,m_country->name);

    message_size = strlen(subdirectory)+1;

    /*Writing path to pipe*/
    write_to_pipe(sizeof(unsigned int) , bufferSize , ((int(*)[2])fd)[i][WRITE] , &message_size );
    write_to_pipe(message_size , bufferSize , ((int(*)[2])fd)[i][WRITE] , subdirectory );


    free(subdirectory);
    i = (++i) % numMonitors;
}

/*Error message for wrong command format and get new command*/
unsigned int wrongFormat_command() {

    printf("ERROR\n");
    while (fgets(lineInput, fileBufferSize, stdin)==NULL || !strcmp(lineInput,"\n"))
        if (!running) terminateProgram();
    strcpy(bufferLine,lineInput);
    token = strtok(lineInput, " \t\n");
    return 1;
}

/*Change value of running to 0 if SIGINT/SIGQUIT occurs*/
void changeStatus_running(int signo) {

    running=0;
}

/*Kill monitors, print logs, free memory and exit*/
void terminateProgram() {

    int stat_val, fd;
    pid_t pid;
    char tempstring[16];

    /*Send SIGKILL to all monitors*/
    for (int i=0 ; i<numMonitors ; i++) {

        kill(monitor_pid[i],SIGKILL);
        pid = wait(&stat_val);
    }

    strcpy(tempstring,"log_file.");
    sprintf(tempstring+9,"%d",getpid());

    if(( fd=open(tempstring ,O_CREAT |O_RDWR |O_TRUNC,0777))==-1) {
        perror("creating ");
        exit(1);
    }

    /*Write all country names*/
    skipList_applyToAll(countriesSkipList,&fd,NULL,NULL,&writeLog);

    /*write the rest of the data*/
    write(fd,"TOTAL TRAVEL REQUESTS ",sizeof(char)*22);
    sprintf(tempstring,"%d",acceptedReq+rejectedReq);
    write(fd,tempstring,strlen(tempstring));
    write(fd,"\n",sizeof(char));

    write(fd,"ACCEPTED ",sizeof(char)*9);
    sprintf(tempstring,"%d",acceptedReq);
    write(fd,tempstring,strlen(tempstring));
    write(fd,"\n",sizeof(char));

    write(fd,"REJECTED ",sizeof(char)*9);
    sprintf(tempstring,"%d",rejectedReq);
    write(fd,tempstring,strlen(tempstring));    

    /*free allocated memory*/
    free(lineInput);
    free(inputDir);
    free(monitor_pid);
    free(pfds);
    skipList_destroy(countriesSkipList);
    for (int i=0 ; i<numMonitors ; i++) 
        if (bloomHashes[i]!=NULL) {
            hash_applyToAllNodes(bloomHashes[i],NULL,&virus_destroy);
            hash_destroy(bloomHashes[i]);
        }
    free(bloomHashes);
    hash_destroy(requestsHash);
    exit(0);
}

/*Write to fd the name of the country given*/
int writeLog(void *vm_country, void* fdPtr, void* data1, void *data2) {

    int fd = *(int*)fdPtr;
    MonitoredCountry *m_country = (MonitoredCountry*) vm_country;

    write(fd,m_country->name,strlen(m_country->name));
    write(fd,"\n",sizeof(char));
    return 0;
}

/*Recreate a child monitor in case of SIGKILL sent to one monitor*/
void recreateChild(int signo){

    /*If SIGKILL came from parent process do nothing*/
    if (!running) return;

    int stat_val,i,targetMonitor=-1;

    pid_t pid = wait(&stat_val);

    /*Locate the monitor to be recreated*/
    for (i=0 ; i<numMonitors ; i++) {

        if (monitor_pid[i]==pid) {
            targetMonitor=i;
            break;
        }
    }
    if (targetMonitor==-1) printf("ERROR oops %d\n",pid);

    /*Create pipes for target monitor*/
    create_pipes_and_monitors(targetMonitor);

    /*Assign properly 1 or more countries to new monitor*/
    skipList_applyToAll(countriesSkipList,fd,&targetMonitor,NULL,&assignCountryToNewChild);

    /*Send _COUNTRIES_END*/
    message_size=15;
    strcpy(tempString,"_COUNTRIES_END");
    write_to_pipe(sizeof(unsigned int) , bufferSize , fd[targetMonitor][WRITE] , &message_size );
    write_to_pipe(message_size , bufferSize , fd[targetMonitor][WRITE] , tempString );

    receive_bloom_filters(targetMonitor);
    return;
}

/*Assign given country to gicen monitor if monitorNum matches*/
int assignCountryToNewChild(void *data1, void *fd, void *vmonitor, void *data4) {

    MonitoredCountry *m_country = (MonitoredCountry*) data1;
    int monitor = *(int*)vmonitor;

    if (m_country->monitorNum!=monitor) return 0;

    /*Construct country directory path*/
    subdirectory = (char*) malloc(strlen(inputDir)+strlen(m_country->name)+2);
    strcpy(subdirectory,inputDir);
    strcat(subdirectory,"/");
    strcat(subdirectory,m_country->name);

    message_size = strlen(subdirectory)+1;

    /*Send path to pipe*/
    write_to_pipe(sizeof(unsigned int) , bufferSize , ((int(*)[2])fd)[monitor][WRITE] , &message_size );
    write_to_pipe(message_size , bufferSize , ((int(*)[2])fd)[monitor][WRITE] , subdirectory );


    free(subdirectory);
}

/*Create 2 pipes and fork 1 monitor*/
void create_pipes_and_monitors(int mon){

    /*Create 2 pipes for read - wrirte for given monitor*/
    sprintf(fifoName[READ], "mon%d_to_tm",mon);
    if ( mkfifo(fifoName[READ], 0666) == -1 ){
        if ( errno!=EEXIST ) { perror("Cant create named pipe!"); exit(6); };
    }
    if ( (fd[mon][READ]=open(fifoName[READ], O_RDWR)) < 0){
        perror("Cant open named pipe!"); exit(3);	
    }

    /*Save reading pipe for poll use*/
    pfds[mon].fd = fd[mon][READ];
    pfds[mon].events = POLLIN;

    sprintf(fifoName[WRITE], "tm_to_mon%d",mon);
    if ( mkfifo(fifoName[WRITE], 0666) == -1 ){
        if ( errno!=EEXIST ) { perror("Cant create named pipe!"); exit(6); };
    }
    if ( (fd[mon][WRITE]=open(fifoName[WRITE], O_RDWR)) < 0){
        perror("Cant open named pipe!"); exit(3);	
    }

    /*Fork the new monitor*/
    pid = fork();
    switch(pid) {

        case -1:
            perror("fork failed\n");
            exit(1);

        case 0:
            execlp("./Monitor",fifoName[READ],fifoName[WRITE],NULL);
    }

    monitor_pid[mon]=pid;

    /*Send buffersize and bloomsize to new monitor*/
    write_to_pipe(sizeof(unsigned int) , bufferSize , fd[mon][WRITE] , &bufferSize );
    write_to_pipe(sizeof(unsigned int) , bufferSize , fd[mon][WRITE] , &sizeOfBloom );
    return;
}

/*Get bloom filters from monitor mon*/
void receive_bloom_filters(int mon) {

    read_from_pipe(sizeof(message_size),bufferSize,fd[mon][READ],&message_size);
    message = malloc(message_size);
    read_from_pipe(message_size,bufferSize,fd[mon][READ],message);

    /*While there are monitors to read*/
    while (strcmp(message,"_BLOOM_END")) {

        /*Insert in system*/

        virus_initialize(&currentVirus,(char*)message);
        free(message);

        /*Initialize proper bloomhash*/
        if (bloomHashes[mon]==NULL) bloomHashes[mon]=hash_Initialize();

        /*Insert virus if it doesnt exist*/
        virusptr = (Virus*) hash_searchValue(bloomHashes[mon],currentVirus.name,&currentVirus,sizeof(Virus),&virus_compare);
        virusptr->bloomFilter = bloomFilter_create(sizeOfBloom);

        /*Read bloom filter*/
        read_from_pipe(sizeof(message_size),bufferSize,fd[mon][READ],&message_size);
        message = malloc(message_size);
        read_from_pipe(message_size,bufferSize,fd[mon][READ],virusptr->bloomFilter);

        free(message);

        read_from_pipe(sizeof(message_size),bufferSize,fd[mon][READ],&message_size);
        message = malloc(message_size);
        read_from_pipe(message_size,bufferSize,fd[mon][READ],message);
    }
    free(message);
}