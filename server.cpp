#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

// Linux syscalls are used here
//
// socket() syscalls returns an fd (integer that refers to something in the linux kernel)
//
// bind() and listen() - associates an address to a socket fd accepts connections to that
// address
//
// accept() takes a listening fd when a client makes a connection and returns an fd that 
// represents a socket
//
// read() recieves data from a TCP connection. 
// write() sends data
// close() destroys
//
//connect() is used on the client side to connect to the server

static void msg(const char *msg){
	fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg){
	int err = errno;
	fprintf(stderr, "[%d] %s\n", err, msg);
	abort();
}

static void do_something(int connfd){
	char rbuf[64] = {};
	ssize_t n = read(connfd, rbuf, sizeof(rbuf) -1);
	if(n<0){
		msg("read() error");
		return;
	}

	printf("client says: %s\n", rbuf);

	char wbuf[] = "world";
	write(connfd, wbuf, strlen(wbuf));
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
			continue;
		}

		do_something(connfd);
		close(connfd);
	}

	return 0;
}
