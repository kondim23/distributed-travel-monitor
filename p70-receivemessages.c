#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MSGSIZE 10

char *fifo = "myfifo";
void read_from_pipe(unsigned int, unsigned int, int, char message[MSGSIZE+1]);

void 	main(int argc, char *argv[]){
	int fd, i, nwrite, bytes_read;
	unsigned short int buffer_size=4, bytes_to_read, message_size, btr_bac, ms_bac;
	char msgbuf[buffer_size], message[MSGSIZE+1]="\0";

	if (argc>2) { printf("Usage: receivemessage & \n"); exit(1); }

	if ( mkfifo(fifo, 0666) == -1 ){
		if ( errno!=EEXIST ) { perror("receiver: mkfifo"); exit(6); };
		}
	if ( (fd=open(fifo, O_RDWR)) < 0){
		perror("fifo open problem"); exit(3);	
		}
	for (;;){

		read_from_pipe(4,buffer_size,fd,message);

		printf("Message Received: %d\n", *(int*)message);
		fflush(stdout);

		message_size = *(int*)message;
		ms_bac = message_size;

		read_from_pipe(message_size,buffer_size,fd,message);

		message[ms_bac] = '\0';
		printf("Message Received: %s\n", message);
		fflush(stdout);
	}
}

void read_from_pipe(unsigned int message_size, unsigned int buffer_size, int fd, char message[MSGSIZE+1]) {

	unsigned int bytes_to_read, btr_bac, bytes_read;
	char msgbuf[buffer_size];

	for (int i=0 ; message_size>0 ; i++) {

		bytes_to_read = message_size>buffer_size ? buffer_size : message_size;
		btr_bac = bytes_to_read;

		do {
			if ( (bytes_read = read(fd, msgbuf, bytes_to_read)) < 0) {
				perror("problem in reading"); exit(5);
			}
			bytes_to_read -= bytes_read;
		} while (bytes_to_read>0);

		message_size -= btr_bac;
		memcpy(message+i*buffer_size,msgbuf,btr_bac);
	}
	return;
}