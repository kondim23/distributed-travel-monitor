#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include  <sys/types.h>
#include  <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {

    unsigned int numMonitors, bufferSize, sizeOfBloom, message_size;
    char *inputDir, *subdirectory, fifoName[2][14];
    int pid, i, nwrite;
    DIR 	*dir_ptr;
    struct 	dirent *direntp;

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
/**/
            message_size = strlen(subdirectory);

            if ((nwrite=write(fd[i][WRITE], &message_size, sizeof(int))) == -1) {
                perror("Error in Writing"); exit(2);
            }
            // printf("%d\n",nwrite);

            if ((nwrite=write(fd[i][WRITE], subdirectory, message_size)) == -1) {
                perror("Error in Writing"); exit(2);
            }
            // printf("%d\n",nwrite);
/**/
            free(subdirectory);
            i = (++i) % numMonitors;
        }
        closedir(dir_ptr);
    }

    free(inputDir);
}