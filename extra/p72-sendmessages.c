#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include "pipe.h"
#define MSGSIZE 10

char *fifo = "myfifo";

void write_to_pipe(unsigned int , unsigned int , int , void* );

void	main(int argc, char *argv[]){
	int fd, i, nwrite, message_size, buffer_size=1000;
	char msgbuf[MSGSIZE+1];

	// if (argc<2) { printf("Usage: sendamessage ... \n"); exit(1); }

	if ( (fd=open(fifo, O_WRONLY| O_NONBLOCK)) < 0)
		{ perror("fife open error"); exit(1); }

	// for (i=1; i<argc; i++){
	// 	// if (strlen(argv[i]) > MSGSIZE){
	// 	// 	printf("Message with Prefix %.*s Too long - Ignored\n",10,argv[i]); 
	// 	// 	fflush(stdout);
	// 	// 	continue;
	// 	// }
	// 	message_size = strlen(argv[i])+1;
	// 	write_to_pipe(sizeof(int),buffer_size,fd,&message_size);

	// 	strcpy(msgbuf,argv[i]);
	// 	write_to_pipe(message_size,buffer_size,fd,msgbuf);
	// }

	void *par=malloc(100000);
	if (par==NULL) printf("error");

	printf("%ld\n",sizeof(*par));

	message_size = 100000;
	write_to_pipe(sizeof(unsigned int),buffer_size,fd,&message_size);

	write_to_pipe(message_size,buffer_size,fd,par);
	exit(0);
}

// void write_to_pipe(unsigned int message_size, unsigned int buffer_size, int fd, void* message) {

// 	unsigned int bytes_to_write, btw_bac, bytes_written, total_written_bytes;
// 	void* msgbuf = malloc(buffer_size);
	
// 	for (int i=0 ; message_size>0 ; i++) {

// 		bytes_to_write = message_size>buffer_size ? buffer_size : message_size;
// 		btw_bac = bytes_to_write;
// 		total_written_bytes = 0;
// 		memcpy(msgbuf,message+i*buffer_size,btw_bac);

// 		do {
// 			if ( (bytes_written = write(fd, msgbuf+total_written_bytes, bytes_to_write)) < 0) {
// 				perror("problem in writing"); exit(5);
// 			}
// 			bytes_to_write -= bytes_written;
// 			total_written_bytes += bytes_written;
// 		} while (bytes_to_write>0);

// 		message_size -= btw_bac;
// 	}
// 	free (msgbuf);
// 	return;
// }