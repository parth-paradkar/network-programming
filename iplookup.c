#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc,  char *argv[]){
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];
	// Check validity of input arguments
	if(argc != 2) {
		fprintf(stderr, "usage: showzip hostname\n");
		return 1;
	}
	// Ensure that hints is empty
	memset(&hints, 0, sizeof(hints));
	// Set the required parameters in hints
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Call getaddrinfo() to get linked list of addrinfo objects
	status = getaddrinfo(argv[1], NULL, &hints, &res);
	if(status != 0) {
		// gai_strerror() => converts status code from getaddrinfo() to string error
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}
	// No error encountered, res obtained
	
	printf("IP address for %s\n\n", argv[1]);
	
	//Looping through res
	for(p = res; p!= NULL; p = p -> ai_next){
		void *addr;
		char *ipver;
		// For IPv4
		if(p -> ai_family == AF_INET) {
			// Type casting into sockaddr_in 
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p -> ai_addr;
			addr = &(ipv4 -> sin_addr);
			ipver = "IPv4";
		} else {
			// For IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p -> ai_addr;
			addr = &(ipv6 -> sin6_addr);
			ipver = "IPv6";
		}
		// Converting to string
		inet_ntop(p -> ai_family, addr, ipstr, sizeof(ipstr));
		printf("  %s: %s\n", ipver, ipstr);

	}

	freeaddrinfo(res);

	return 0;
}
