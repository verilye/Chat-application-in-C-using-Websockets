#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

// Note - apparently snake case is used a lot in linux os, maybe use that

// Protocol implementation
// For simplicity Im going to use the protocol example that was in the tutorial book
// 4 digit msg size number then message
// msgsize - msg - msgsize - msg

static const int MAX_BUFFER_SIZE = 4096;


static int write_buffer(int fd, ssize_t buffer, int size){


	return 0;
}

static int read_buffer(int fd, ssize_t buffer, int size){

	while(size>0){
	

	}

	return 0;
}

int main(){

	// Documentation states that 0 can be used if only one protocl is used
	
	// Open socket connection using IPv6
	int fd = socket(AF_INET6, SOCK_STREAM, 0);

	// handle error
	if(fd == -1){
		printf("%d", errno);
		return -1;
	}

	ssize_t buffer[MAX_BUFFER_SIZE - 1];

	while(true){

		// Find size of message
		ssize_t header = read(fd, buffer, 4);

		if(header > 0){

			// read through buffer and get message size from header			


		}
	}


	return 0;
}
