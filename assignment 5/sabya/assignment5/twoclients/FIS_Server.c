/*
** FIS_Server.c -- a datagram sockets "server" 
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

#define MYPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 100

typedef struct indexelement {
	char filename[80];
	char hostname[80];
}indexelement;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* gethost(const char* filename, indexelement* indexfile, int indexsize) {
	int i;
	char* reply = malloc(80);
	strcpy(reply,"");
	for(i = 0; i < indexsize; ++i) {
		if(!strcmp(indexfile[i].filename, filename)) {
			if(strlen(reply) > 0)strcat(reply, ",");
			strcat(reply, indexfile[i].hostname);
		}
	}
	return reply;
}

int main(int argc, char* argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv, numbytes, yes = 1, len = 0, indexsize = 10, i = 0;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN], s[INET6_ADDRSTRLEN], *line = NULL, *tofree, *token, *reply = NULL;
	socklen_t addr_len;
	indexelement* indexfile;
	FILE* file;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
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

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	file = fopen("fileinfo", "r");
	indexfile = (indexelement*)malloc(sizeof(indexelement) * indexsize);
	while (getline(&line, (unsigned long*)&len, file) > 0) {
		tofree = line;
		token = strsep(&line,",\n");
		strcpy(indexfile[i].filename, token);
		token = strsep(&line,",\n");
		strcpy(indexfile[i].hostname, token);
		i++;
		free(tofree);
		line = NULL;
		len = 0;
	}
	indexsize = i;

	while(1) {
		
		printf("server: waiting to recvfrom...\n");
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		buf[numbytes] = '\0';
		printf("FIS has got file %s request from %s\n", buf,
			inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));

		reply = gethost(buf, indexfile, indexsize);
		if(!strcmp(reply, ""))strcpy(reply, "none");

		if ((numbytes = sendto(sockfd, reply, strlen(reply), 0,
			 (struct sockaddr *)&their_addr, addr_len)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		printf("FIS has sent reply %s to %s\n", reply,
			inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		free(reply);
	}

}