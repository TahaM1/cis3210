#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAXRCVLEN 500
#define PORTNUM 2300
 
int main(int argc, char *argv[])
{
	char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
	char* msg = "Hello Worlds!\n";
	int len, mysocket;
	struct sockaddr_in dest; // socket info about the machine connecting to us
 
	/* Create a socket.
	   The arguments indicate that this is an IPv4, TCP socket
	*/
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
  
	memset(&dest, 0, sizeof(dest));                // zero the struct
	
	//Initialize the destination socket information
	dest.sin_family = AF_INET;					   // Use the IPv4 address family
	if (argc == 1){
		dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Set destination IP number - localhost, 127.0.0.1
	}else if (argc == 2){
		inet_aton(argv[1], &dest.sin_addr); // Set destination IP number from command line arg
	}else{
		printf("Invalid number of arguments\n");
		printf("Usage: %s <ip>, where ip is an optional IP4 address in the dot-decimal notation\n", argv[0]);
		printf("If the IP address is not proided, client attempts to connect to localhost\n");
		exit(0);
	}

	dest.sin_port = htons(PORTNUM);                // Set destination port number
 	
	// Connect to the server
	connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
  
	send(mysocket, msg, strlen(msg), 0); 
	len = recv(mysocket, buffer, MAXRCVLEN, 0);
 
	/* We have to null terminate the received data ourselves */
	buffer[len] = '\0';
 
	printf("Received %s (%d bytes) from %s on port %d.\n", buffer, len, inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));
 
	close(mysocket);
	return EXIT_SUCCESS;
}
