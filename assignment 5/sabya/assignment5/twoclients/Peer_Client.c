/*
** Peer_Client.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>

#define SERVERPORT "4950"	// the port users will use to connect to the FIS Server
#define PORT "3490"			// the port users will use to connect to a client to download a file

int Socket;
struct addrinfo *garbage;

// returns 1 if file is present in shared folder , else returns 0
int ispresent(const char* filename) {
	char path[100];
	strcpy(path, "shared1/");
	strcat(path, filename);
	if (access(path, F_OK) != -1) return 1;
	return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// signal handler for SIGINT
void sighandler(){
	freeaddrinfo(garbage);
	close(Socket);
	exit(0);
}


int main(int argc, char *argv[])
{
	int sockfd, sockfd2;
	struct addrinfo hints, *servinfo, *servinfo2, *p, *p2;
	int rv, numbytes, numfiles = 0, i, choice, fd;
	char s[INET6_ADDRSTRLEN], filename[80], buf[80], *token, *temp, *tofree, *list[10], path[100];

	// register signal handler
	signal(SIGINT, sighandler);

	// give server IPV4 address as commandline argument
	if (argc != 2) {
		fprintf(stderr,"usage: ./client hostname\n");
		exit(1);
	}

	// get a datagram socket, initialize addrinfo structure
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;

	// get addrinfo structure list
	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}

	// assign socket and addrinfo list so that they can be freed later
	Socket = sockfd;
	garbage = servinfo;

	// client enters a loop
	while(1) {
	
		// Enter filename to download
		printf("Enter filename : ");
		scanf("%s", filename);

		// check if file is present already in shared folder
		// if yes reiterate
		if(ispresent(filename)) {
			printf("File already available in shared folder\n");
			continue;
		}

		// send file IP request to FIS
		if ((numbytes = sendto(sockfd, filename, strlen(filename), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
			perror("client: sendto");
			exit(1);
		}
		printf("client: sent file %s request (%d bytes) to %s\n", filename, numbytes, argv[1]);

		// get file IP information from server
		if ((numbytes = recvfrom(sockfd, buf, 80, 0,
			p->ai_addr, &(p->ai_addrlen))) == -1) {
			perror("recvfrom");
			exit(1);
		}
		buf[numbytes] = '\0';
		
		// if file not available with any user reiterate
		if(!strcmp(buf, "none")){
			printf("No user to download from...\n");
			continue;
		}

		// divide the IP list obtained into IPs
		temp = (char*)malloc(80);
		strcpy(temp, buf);
		tofree = temp;
		numfiles = 0;
		while ((token = strsep(&temp,",")) != NULL) {
			list[numfiles++] = token;
		}
		for (i = 0; i < numfiles; ++i){
			printf("%2d : %s\n", i+1, list[i]);
		}
		free(tofree);

		// ask user to enter a user IP
		printf("Choose a user...(Enter a serial number) : ");
		scanf("%d", &choice);

		// go on asking the user till a valid address is not available
		while(choice < 0 || choice > numfiles){
			printf("Enter a valid choice... : ");
			scanf("%d", &choice);
		}

		// obtain a STREAM Socket for downloading the file
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		// get the struct addrinfo list
		printf("%s\n",list[choice-1]);
		if ((rv = getaddrinfo(list[choice-1], PORT, &hints, &servinfo2)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and connect to the first we can
		for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
			if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
					p2->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}

			if (connect(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
				close(sockfd2);
				perror("client: connect");
				continue;
			}

			break;
		}
		if (p2 == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			return 2;
		}

		inet_ntop(p2->ai_family, get_in_addr((struct sockaddr *)p2->ai_addr),
				s, sizeof s);
		printf("client: connecting to %s\n", s);

		freeaddrinfo(servinfo2); // all done with this structure

		// send the filename to the client
		if(send(sockfd2, filename, sizeof(filename), 0) == -1) {
			perror("client : send");
			exit(1);
		}
		
		// open a file to write the received data
		strcpy(path, "downloaded/");
		strcat(path, filename);
		fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);

		// reveive bytes from server as long as the connection is open
		while ((numbytes = recv(sockfd2, buf, 80, 0)) != 0) {
			write(fd, buf, numbytes);
		}

		// close the file descriptor and socket
		close(fd);
		close(sockfd2);
		printf("File downloaded successfully...\n");

	}

	freeaddrinfo(servinfo);
	close(sockfd);

	//return 0;
}