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


// Combination of linux syscalls and the socket library used to create 
// connection and bind sockets to linux fd's

const size_t k_max_msg = 4096;


// spits the message out so that its readable
static void msg(const char *msg){
	fprintf(stderr, "%s\n", msg);
}

// Safely exit the program - the program dies
static void die(const char *msg){
	int err = errno;
	fprintf(stderr, "[%d] %s\n", err, msg);
	abort();
}

// Reads from the kernel fd representing the connection
static int32_t read_full(int fd, char *buf, size_t n){
	
	while(n>0){
	
		ssize_t rv = read(fd,buf,n);
		if(rv <=0){
			return -1;
		}

		assert((size_t)rv <=n);
		n -= (size_t)rv;
		buf += rv;

	}

	return 0;
}

// Writes data back to the kernel fd representing the connection
static int32_t write_all(int fd, const char *buf, size_t n){
	while(n>0){
		ssize_t rv = write(fd, buf, n);
		
		if(rv<=0){
			return -1;
		}

		assert((size_t)rv <= n);

		n-=(size_t)rv;

		buf+=rv;

	}

	return 0;
}

static int32_t one_request(int connfd){
	
	// 4 bytes header
	char rbuf[4 + k_max_msg + 1];
	
	errno = 0;
	
	int32_t err = read_full(connfd, rbuf, 4);
	
	if(err){
		if(errno == 0){
			msg("EOF");
		}else{
			msg("read() error");
		}

		return err;
	}

	uint32_t len = 0;

	//copy from memory to buffer
	memcpy(&len, rbuf, 4); // assume little endian
	
	if(len > k_max_msg){
		msg("too long");
		return -1;
	}

	// request body
	err = read_full(connfd, &rbuf[4], len);

	if(err) {
		msg("read() error");
		return err;
	}

	// do something
	
	rbuf[4 + len] = '\0';
	printf("client says: %s\n", &rbuf[4]);


	// reply using the same protocol
	
	const char reply[] = "world";
	char wbuf[4 + sizeof(reply)];
	len = (uint32_t)strlen(reply);
	memcpy(wbuf, &len, 4);
	memcpy(&wbuf[4], reply, len);
	
	return write_all(connfd, wbuf, 4 + len);

}


int main(){
	// AF_INET is for IPv4, SOCK_STREAM is for TCP
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	int val = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	
	// bind, this is the syntax that deals with IPv4 addresses
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(0);	//wildcard address 0.0.0.0
	int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));

	if(rv){
		die("bind()");
	}

	// listen
	rv = listen (fd, SOMAXCONN);
	if(rv){
		die("listen()");
	}

	while(true){
		// accept
		struct sockaddr_in client_addr = {};

		socklen_t socklen = sizeof(client_addr);
		
		int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
		if(connfd <0){
			continue; // error
		}
		
		//Read through one request using the linux system code tied to connfd
		while(true){
			int32_t err = one_request(connfd);

			if(err){
				break;
			}

		}
		close(connfd);
	}

	return 0;
}
