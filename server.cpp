


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

int main(){
	// AF_INET is for IPv4, SOCK_STREAM is for TCP
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	int val = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));


}
