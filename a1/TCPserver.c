#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORTNUM 2300
#define MAXRCVLEN 500
#define MAX_FILE_NAME 255
#define DEFAULT_BUFSIZE 4096

int mysocket; // socket used to listen for incoming connections
int consocket;

static void sigintCatcher(int signal, siginfo_t *si, void *arg)
{
	printf("\n\n************** Caught SIG_INT: shutting down the server ********************\n");

	close(consocket);
	close(mysocket);
	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s port-number [bufSize]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int serverPort = atoi(argv[1]);
	int bufSize = (argc == 3) ? atoi(argv[2]) : DEFAULT_BUFSIZE;

	int len;
	char buffer[bufSize + 1]; // +1 so we can add null terminator

	struct sockaddr_in dest; // socket info about the machine connecting to us
	struct sockaddr_in serv; // socket info about our server

	// set up the sigint handler
	struct sigaction signaler;

	memset(&signaler, 0, sizeof(struct sigaction));
	sigemptyset(&signaler.sa_mask);

	signaler.sa_sigaction = sigintCatcher;
	signaler.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &signaler, NULL);

	// Set up socket info
	socklen_t socksize = sizeof(struct sockaddr_in);

	memset(&serv, 0, sizeof(serv));			  // zero the struct before filling the fields
	serv.sin_family = AF_INET;				  // Use the IPv4 address family
	serv.sin_addr.s_addr = htonl(INADDR_ANY); // Set our address to any interface
	serv.sin_port = htons(serverPort);		  // Set the server port number

	/* Create a socket.*/
	mysocket = socket(AF_INET, SOCK_STREAM, 0);

	int flag = 1;

	if (setsockopt(mysocket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
	{
		printf("setsockopt() failed\n");
		printf("%s\n", strerror(errno));
		exit(1);
	}

	if (bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr)) != 0)
	{
		printf("Unable to open TCP socket on localhost:%d\n", serverPort);

		printf("%s\n", strerror(errno));
		close(mysocket);
		return 0;
	}

	// start listening, allowing a queue of up to 10 pending connection
	listen(mysocket, 10);

	// Create a socket to communicate with the client that just connected
	consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);

	while (consocket)
	{

		FILE *file;
		char filename[MAX_FILE_NAME];
		char file_size[3];
		long file_total_size = 0;
		int chunks = 0;
		// Receive the filename from the client
		printf("=========================================\n");
		len = recv(consocket, file_size, 4, 0);

		len = recv(consocket, filename, atoi(file_size), 0);
		filename[atoi(file_size)] = '\0';

		// Open a file with the received filename for writing
		file = fopen(filename, "wb");
		if (file == NULL)
		{
			perror("Error opening file for writing");
			close(mysocket);
			return EXIT_FAILURE;
		}
		printf("Incoming connection from %s on port %d\n", inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));

		// Receive data from the client
		while ((len = recv(consocket, buffer, bufSize, 0)) > 0)
		{
			buffer[len] = '\0';
			file_total_size += len;
			chunks++;
			// Write received data to the file
			fwrite(buffer, 1, len, file);
		}

		if (len == -1)
		{

			printf("An error occured in the transport of the file.\n");
		}
		printf("Filename: %s\nFile size: %d\nChunks: %d\n", filename, file_total_size, chunks);

		fclose(file);
		// Close current connection
		close(consocket);

		// Continue listening for incoming connections
		consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
	}

	close(mysocket);

	return EXIT_SUCCESS;
}
