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

void 	main(int argc, char *argv[]){
	int fd, i, nwrite, bytes_read;
	unsigned short int buffer_size=4, bytes_to_read, message_size;
	char msgbuf[buffer_size+1], message[MSGSIZE+1]="\0";

	if (argc>2) { printf("Usage: receivemessage & \n"); exit(1); }

	if ( mkfifo(fifo, 0666) == -1 ){
		if ( errno!=EEXIST ) { perror("receiver: mkfifo"); exit(6); };
		}
	if ( (fd=open(fifo, O_RDWR)) < 0){
		perror("fifo open problem"); exit(3);	
		}
	for (;;){
		message_size = MSGSIZE+1;
		// bytes_to_read = MSGSIZE+1;
		while (message_size>1) {
			bytes_to_read = message_size>buffer_size ? buffer_size : message_size;
			message_size -= bytes_to_read;
			msgbuf[bytes_to_read]='\0';
			do {
				if ( (bytes_read = read(fd, msgbuf, bytes_to_read)) < 0) {
					perror("problem in reading"); exit(5);
				}
				bytes_to_read -= bytes_read;
			} while (bytes_to_read>0);
			strcat(message,msgbuf);
		}
		printf("Message Received: %s\n", message);
		message[0]='\0';
		fflush(stdout);
	}
}