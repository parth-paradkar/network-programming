#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT "7000"
#define BACKLOG 10
#define MAXDATASIZE 100

void sigchld_handler(int s){
	// errno => number of the last error
	int saved_errno = errno;
	// waitpid() => suspends execution of the calling process until a child specified by pid argument has changed state
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
	if(sa -> sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa) -> sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa) -> sin6_addr);
}

int main(){
	int sockfd, new_fd;
	struct addrinfo hints, *res, *p;
	int status;
	socklen_t sin_size;
	struct sockaddr_storage their_addr; // client's information
	char s[INET6_ADDRSTRLEN];
	int yes = 1; // Flag for socket options
	struct sigaction sa; // info about how to handle signals
	char *msg = "Hello World!";
	int msg_len = strlen(msg);
	int bind_status;
	char buf[MAXDATASIZE];
	int numbytes;
	memset(&hints, 0, sizeof(hints));

	// Set hints params
	hints.ai_family = AF_UNSPEC; // Any
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // Needed for bind()
	
	// getaddrinfo() => Get a linked list of addrinfo objects with the parmas specified in hints
	status = getaddrinfo(NULL, PORT, &hints, &res);
	
	if(status != 0) {
		// gai_strerror() => Convert status code from getaddrinfo() to string error
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}
	
	for(p = res; p != NULL; p = p -> ai_next){
		// socket() => Get a socket descriptor with the given specifications
		sockfd = socket(p -> ai_family, p-> ai_socktype, p-> ai_protocol);
		if(sockfd == -1){
			perror("server: socket");
			// Go to next addrinfo struct in res
			continue;
		}
		
		// Set socket options to allow reuse of local address
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			// Handle error
			perror("setsockopt");
			exit(1);
		}
		
		// bind() => Assign address to socket
		bind_status = bind(sockfd, p -> ai_addr, p-> ai_addrlen);
		
		// Handle error
		if(bind_status == -1){
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}
	
	// Free up structs (no longer needed)
	freeaddrinfo(res);

	// If none of the structs worked i.e. loop reached the end of linked list
	if(p == NULL){
		printf("failed to bind!\n");
		exit(1);
	}
	
	// listen() => listen for connection on the specified port

	if(listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	 // remove dead processes
	sa.sa_handler = sigchld_handler; // pointer to a signal handling function that Specifies action to be take on recieveing signal
	
	// Collection of signals that are blocked => signal mask
	// sigemptyset() => Initialize empty set of signals
	sigemptyset(&sa.sa_mask);
	
	sa.sa_flags = SA_RESTART;
	
	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	printf("Listening in port %s..\n", PORT);
	while(1){
		sin_size = sizeof(their_addr);
		// Creating a new thread for the new connection => runs the same code
		// accept() => Returns a new socket descriptor
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd == -1) {
			perror("accept");
			continue;
		}

		// Getting client information, saving to s
		
		// ss_family of sockaddr_storage => sa_family of sockaddr

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s)); // casting sockaddr_storage to sockaddr
		printf("Got connection from %s\n", s);

		// fork() => create child process, return -1 if failed

		if(!fork()){ // This is the child process
			// Close the socket that was created again
			close(sockfd);
			// Define the behaviour of server on getting a connection for an independent process
			while(1){
				if((numbytes = recv(new_fd, buf, MAXDATASIZE-1 ,0)) == -1){
                                	perror("recv");
                                	exit(1);
				}
				buf[numbytes] = '\0';
				printf("Message recieved: %s\n", buf);
				if(strcmp(buf, "bye")){
					printf("Closing connection to %s\n", s);
					break;
				}
                        }
			
			// Close the child process
			close(new_fd);
			exit(0);
		}
		// Not necessary for the parent process
		close(new_fd);
	}
	return 0;
}
