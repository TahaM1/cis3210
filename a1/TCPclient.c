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
#define DEFAULT_BUFSIZE 4096

void error(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s fileName server-IP-address:port-number [bufSize]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	char *fileName = argv[1];
	char *serverAddress = strtok(argv[2], ":");
	char *serverPortStr = strtok(NULL, ":");
	int serverPort = (serverPortStr != NULL) ? atoi(serverPortStr) : PORTNUM;
	int bufSize = (argc == 5) ? atoi(argv[4]) : DEFAULT_BUFSIZE;

	FILE *file = fopen(fileName, "rb");
	if (file == NULL)
	{
		error("Error opening file");
	}

	char buffer[bufSize];
	int bytesRead;

	int mysocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mysocket == -1)
	{
		error("Error creating socket");
	}

	struct sockaddr_in dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(serverPort);

	if (inet_pton(AF_INET, serverAddress, &(dest.sin_addr)) <= 0)
	{
		error("Error converting IP address");
	}

	if (connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in)) == -1)
	{
		error("Error connecting to the server");
	}

	// Send the file in chunks
	while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0)
	{
		if (send(mysocket, buffer, bytesRead, 0) == -1)
		{
			error("Error sending data");
		}
	}

	printf("File sent successfully.\n");

	fclose(file);
	close(mysocket);

	return EXIT_SUCCESS;
}
