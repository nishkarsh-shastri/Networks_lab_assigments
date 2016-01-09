/*
** Peer_Server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#define PORT "3490"  // the port users will be connecting to
#define DATALEN 512

#define BACKLOG 10	 // how many pending connections queue will hold

int Socket;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// function to send file on socket sockfd
// returns 0 on success, -1 on failure
int send_file(int sockfd, const char* filename){

	int fd = open(filename, O_RDONLY);
	char *line = (char*)malloc(80);
	int bytesread, bytestosend, bytessent, nbytes;

	while((bytesread = read(fd, (void*)line, 80)) > 0) {
		bytestosend = bytesread;
		bytessent = 0;
		while(bytessent < bytestosend) {
			nbytes = send(sockfd, line + bytessent, bytestosend - bytessent, 0);
			if(nbytes == -1) {
					close(fd);
					return -1;
			}
			bytessent += nbytes;
		}
	}

	close(fd);
	if(bytesread == -1)return -1;
	return 0;
}


// signal handler for SIGINT
void sighandler() {
	close(Socket);
	exit(0);
}

int main(int argc, char* argv[])
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	int yes=1, i;
	char s[INET6_ADDRSTRLEN], filename[80], path[80];
	int rv, nChilds = 0;

	// array of CHILD PIDS
	pid_t childs[BACKLOG], childpid;

	// register SIGINT with sighandler
	signal(SIGINT, sighandler);
	for (i = 0; i < BACKLOG; ++i )childs[i] = 0;

	// initialise addrinfo structure for STREAM Socket
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	// get addrinfo list 
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	Socket = sockfd;
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}


	printf("server: waiting for connections...\n");

	sin_size = sizeof their_addr;

	while (1) {

		// wait for an incoming connection
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			return 1;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		// fork a new process to handle the connection
		if ((childpid = fork()) == 0) {

			close(sockfd);
			// reveive filename for downloading
			if (recv(new_fd, filename, sizeof(filename), 0) == -1) {
				perror("Error : filename receive error...");
				exit(1);
			}

			// get full pathname
			//printf("%s\n", filename);
			strcpy(path, "shared/");
			strcat(path, filename);

			// send file to client
			if(send_file(new_fd, path) == -1) {
				perror("Error: filename send error...");
				close(new_fd);
				close(sockfd);
				exit(1);
			}

			close(new_fd);
			exit(0);

		}
		else {

			close(new_fd);
			// store childpid in the first available pid slot
			/*for (i = 0; i < BACKLOG; ++i) {
				if (childs[i] == 0) {
					childs[i] = childpid;
					break;
				}
			}*/
		}

	}
	
	close(sockfd);

	return 0;
}
