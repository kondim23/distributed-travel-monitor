#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void read_from_pipe(unsigned int message_size, unsigned int buffer_size, int fd, void* message) {


	unsigned int bytes_to_read, btr_bac, bytes_read_total;
	int bytes_read;
	void* msgbuf = malloc(buffer_size);


	for (int i=0 ; message_size>0 ; i++) {

		bytes_to_read = message_size>buffer_size ? buffer_size : message_size;
		btr_bac = bytes_to_read;
		bytes_read_total = 0;

		do {
			while ( (bytes_read = read(fd, msgbuf+bytes_read_total, bytes_to_read)) < 0) {}
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

	unsigned int bytes_to_write, btw_bac, total_written_bytes;
	int bytes_written;
	void* msgbuf = malloc(buffer_size);
	
	for (int i=0 ; message_size>0 ; i++) {

		bytes_to_write = message_size>buffer_size ? buffer_size : message_size;
		btw_bac = bytes_to_write;
		total_written_bytes = 0;
		memcpy(msgbuf,message+i*buffer_size,btw_bac);

		do {
			while ( (bytes_written = write(fd, msgbuf+total_written_bytes, bytes_to_write)) < 0) {}
			bytes_to_write -= bytes_written;
			total_written_bytes += bytes_written;
		} while (bytes_to_write>0);

		message_size -= btw_bac;
	}
	free (msgbuf);
	return;
}