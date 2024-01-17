#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>

// For simplicity Im going to use the protocol example that was in the tutorial book
// 4 digit msg size number then message
// msgsize - msg - msgsize - msg

#define PORT "3490" 	// the port users will be connected to 
#define BACKLOG 10 	// How many connection the queue will hold

static const int MAX_BUFFER_SIZE = 4096;

void sigchld_handler(int s){
	// waitpid() might overwrite errno so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(){
	// We use getaddrinfo to bind to the first port that fulfils our requirements
	// that is valid. We can dispose of it immediately after. 
	
	int status;
	
	// This will be information about the selected node in the returned linked list
	struct addrinfo hints, *servinfo, *step; // will point to results
	
	struct sockaddr_storage their_addr;	// connectors address information			 
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int sock_fd, new_fd;
	int rv;
				   
	memset(&hints, 0, sizeof(hints)); // Set all values in hints to 0
	hints.ai_family = AF_UNSPEC; // Unspecified whether IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // Socket type connections
	hints.ai_flags = AI_PASSIVE; // handle IP for me
				     
	if((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}

	// loop through returned linked list and look for valid port to bind fd to
	for(step = servinfo; step!=NULL; step = step->ai_next){
		if((sock_fd = socket(step->ai_family, step->ai_socktype, step->ai_protocol)) == -1){
			perror("server: socket");
			continue;
		}

		if( setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}

		if(bind(sock_fd, step->ai_addr, step->ai_addrlen) == -1){
			close(sock_fd);
			perror("server:bind");
			continue;

		}

		break;
	}

	freeaddrinfo(servinfo); // free up memory once finished using
	
	// check to see if last loop failed
	if(step == NULL){
		fprintf(stderr, "server:failed to bind\n");
		exit(1);
	}
	
	// attempt to listen()
	if(listen(sock_fd, BACKLOG) == -1){
		perror("listen");
		exit(1);
	}

	// reap all dead processes
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}
	
	printf("server: waiting for connections... \n");

	while(true){ // accept() connections
		sin_size = sizeof their_addr;
		new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd == -1){
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from%s\n", s);

		if(!fork()){
			close(sock_fd);
			if(send(new_fd, "Hello, world!\n", 12, 0) == -1){
				perror("send");
			}
			close(new_fd);
			exit(0);
		}

		close(new_fd);
	}


	return 0;
}
