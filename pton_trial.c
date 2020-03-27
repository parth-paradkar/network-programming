#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <inttypes.h>

int main(){
	struct sockaddr_in sa;
	inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr));
	printf("%" PRIu32 "\n", sa.sin_addr.s_addr) ; // => 963513354 == dec( 00111001 01101110 00001100 00001010 )
	printf("%" PRIu32 "\n", htonl(sa.sin_addr.s_addr)); // => 168586809 == dec( 00001010 00001100 01101110 00111001 ) 
	return 0;
}
