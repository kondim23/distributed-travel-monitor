#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include  <sys/types.h>
#include  <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void read_from_pipe(unsigned int , unsigned int , int , void* );

int main(int argc, char *argv[]) {

    unsigned int numMonitors, bufferSize, sizeOfBloom, message_size;
    char *inputDir, *subdirectory, fifoName[2][14], tempString[15];
    int pid, i, nwrite;
    DIR 	*dir_ptr;
    struct 	dirent *direntp;
    void *message;

    enum{READ,WRITE};

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
    int fd[numMonitors][2];

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
    }

    /*Sharing countries subfolders to Monitors*/

    if ((dir_ptr = opendir(inputDir)) == NULL)

        fprintf(stderr, "cannot open %s \n",inputDir);

    else {

        i=0;
        while ((direntp = readdir(dir_ptr)) != NULL) {

            if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) continue;

            subdirectory = (char*) malloc(strlen(inputDir)+strlen(direntp->d_name)+2);
            strcpy(subdirectory,inputDir);
            strcat(subdirectory,"/");
            strcat(subdirectory,direntp->d_name);
            // printf("%s %s %ld\n",direntp->d_name, subdirectory, strlen(subdirectory));

            message_size = strlen(subdirectory)+1;

            if ((nwrite=write(fd[i][WRITE], &message_size, sizeof(unsigned int))) == -1) {
                perror("Error in Writing"); exit(2);
            }
            // printf("%d\n",nwrite);

            if ((nwrite=write(fd[i][WRITE], subdirectory, message_size)) == -1) {
                perror("Error in Writing"); exit(2);
            }
            // printf("%d\n",nwrite);

            free(subdirectory);
            i = (++i) % numMonitors;
        }
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
            free(message);

            read_from_pipe(sizeof(message_size),bufferSize,fd[i][READ],&message_size);
            message = malloc(message_size);
            read_from_pipe(message_size,bufferSize,fd[i][READ],message);
		    printf("Travel Received: %s\n", (char*)message);
        }
        free(message);
    }


    sleep(5);
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