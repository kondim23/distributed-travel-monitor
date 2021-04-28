#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#define MSGSIZE 10

char *fifo = "myfifo";

void	main(int argc, char *argv[]){
	int fd, i, nwrite, message_size;
	char msgbuf[MSGSIZE+1];

	if (argc<2) { printf("Usage: sendamessage ... \n"); exit(1); }

	if ( (fd=open(fifo, O_WRONLY| O_NONBLOCK)) < 0)
		{ perror("fife open error"); exit(1); }

	for (i=1; i<argc; i++){
		// if (strlen(argv[i]) > MSGSIZE){
		// 	printf("Message with Prefix %.*s Too long - Ignored\n",10,argv[i]); 
		// 	fflush(stdout);
		// 	continue;
		// }
		message_size = strlen(argv[i]);

		if ((nwrite=write(fd, &message_size, sizeof(int))) == -1) {
			perror("Error in Writing"); exit(2);
		}
		printf("%d\n",nwrite);

		strcpy(msgbuf,argv[i]);

		if ((nwrite=write(fd, msgbuf, message_size)) == -1) {
			perror("Error in Writing"); exit(2);
		}
		printf("%d\n",nwrite);
	}
	exit(0);
}