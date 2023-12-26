#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>


const size_t k_msg_max= 4096;

static void die(const char *msg){
	int err = errno;
	fprintf(stderr, "[%d] %s\n", err, msg);
	abort();
}

static int32_t write_all(int fd, char * buf, size_t length){
	
	// it appears that the loop is required here because
	// it is possible for the write operation to be interupted
	//
	// getting the return value of write here shows us what data was sent
	
	while(length > 0){
		
		// write to the fd, store the return value so that the rest can be sent
		// if necessary

		ssize_t rv = write(fd, buf, length);
		if(rv<=0){
			return -1; //error
		}

		assert((size_t)rv <= length);
		// assert to be a good boy

		length -= (size_t)rv;
		
		buf += rv;

	}

	return 0;
	
}

static int32_t read_full(int fd, char* buf, size_t n) {

	while(n>0){
		
		ssize_t rv = read(fd, buf, n);

		if(rv <= 0){

			return -1;
		}

		assert((size_t)rv <= n);

		n -= (size_t)rv;
		buf += rv;

	}

	return 0;
}

static int32_t query(int fd, const char *text){
	
	// set length of query, check to make sure its the right size
	
	int32_t len = (int32_t)strlen(text);

	if(len> k_msg_max){
		return -1;
	}

	// copy from text variable to query, append to the end of the header and write to fd
	
	char wbuf[4+ k_msg_max];
	//copy size represented using 4 bits
	memcpy(wbuf,&len,4);
	//copy message up to max msg size after the 4 bit header
	memcpy(&wbuf[4],text,len);
	// write to fd using the write_all function
	if(int32_t err = write_all(fd, wbuf, 4+len)){
		return err;
	}

	// copy from fd and read all input from server
	char rbuf[4 + k_msg_max + 1];
	errno = 0;
	int32_t err = read_full(fd,rbuf,4);
	if(err){
		if(errno ==0){
			msg("EOF");
		} else{
			msg("read() error()")
		}

		return err;
	}

	// print to console

}

int main(){

	// AF_INET is for IPv4 socket connections, IF_INET6 can be used for IPv6
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	// fd is of course a number associated with a process in the linux kernel
	if (fd <0){
		die("socket()");
	}


	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); //127.0.0.1
	int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
	if(rv){
		die("connect");
	}

	char msg[] = "hello";
	write(fd, msg, strlen(msg));

	char rbuf[64] = {};
	ssize_t n = read(fd, rbuf, sizeof(rbuf) -1);
	if(n <0){
 		die("read");
	}
	 
	printf("server says: %s\n", rbuf);
	close(fd);
	return 0;


}
