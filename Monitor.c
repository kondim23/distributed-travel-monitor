#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include  <sys/types.h>
#include  <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

void read_from_pipe(unsigned int, unsigned int, int, char*);

int main(int argc, char *argv[]) {

    int fd[2];
    unsigned int buffer_size = 100, message_size;
    char message[100];

    enum{READ,WRITE};

    /*Check Arguments*/

    if (argc!=2) { printf("Error in Monitor arguments\n"); exit(1); }

	if ( (fd[READ]=open(argv[1], O_RDWR)) < 0)
		{ perror("fife open error"); exit(1); }
    if ( (fd[WRITE]=open(argv[0], O_WRONLY| O_NONBLOCK)) < 0)
		{ perror("fife open error"); exit(1); }


    /*Get Countries Directory from pipe*/

    read_from_pipe(4,buffer_size,fd[READ],message);
    // printf("Message Received: %d\n", *(int*)message);
    // fflush(stdout);
    message_size = *(int*)message;
    read_from_pipe(message_size,buffer_size,fd[READ],message);
    // message[message_size] = '\0';
    // printf("Message Received: %s\n", message);

    fflush(stdout);
}

void read_from_pipe(unsigned int message_size, unsigned int buffer_size, int fd, char* message) {

	unsigned int bytes_to_read, btr_bac, bytes_read, bytes_read_total;
	char msgbuf[buffer_size];

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
	return;
}