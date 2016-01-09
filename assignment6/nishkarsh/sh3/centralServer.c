#include <stdio.h>
#include <iostream>
#include <cerrno>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "myheader.hxx"
#include <iterator>

using namespace std;

#define CENTRALPORT "15555"

#define MAXBUFLEN 100
#define MAXTOKENS 20


bool compareNode(Node *a,Node *b)
{
	return (a->getMachineId()<=b->getMachineId());
}

//The vector containing the node elements
std::vector<Node*> node_list;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv, numbytes, yes = 1, len = 0, indexsize = 10, i = 0;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN], s[INET6_ADDRSTRLEN], *line = NULL, *tofree, *token, *reply = NULL;
	socklen_t addr_len;
	indexelement* indexfile;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL,CENTRALPORT, &hints, &servinfo)) != 0) {
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

	free addrinfo(servinfo);

	while(1)
	{
		printf("central server: waiting to recvfrom...\n");	
		//Here I am assuming that central server is waiting to get a string starting with NEW
		addr_len = sizeof(their_addr);
		if((numbytes=recvfrom(sockfd,buf,MAXBUFLEN-1,0,(struct sockaddr*)&their_addr,&addr_len))==-1)
		{
			perror("recvfrom");
			exit(1);
		}
		printf("listener got packet from %s\n",inet_ntop(their_addr.ss_family),get_in_addr((struct sockaddr*)&their_addr),s,sizeof s);
		printf("listener packet is %d bytes long \n",numbytes);
		buf[numbytes]='\0';

		int numberOfTokens=0;
		string tokenlist[MAXTOKENS];
		char *pch;
		//I am assuming buf to be the string reply from client
		printf("The server received %s\n",buf);
		string rec;
		strcpy(rec,buf);

		string node_ip;
		
		pch = strtok(rec,"-");
		
		while(pch!=NULL)
		{
			tokenlist[numberOfTokens++]=strdup(pch);
			pch = strtok(NULL,"-");
		}

		if(tokenlist[0]=="NEW")
		{
			//A new node has sent request.
			//Its IP is tokenlist[1]
			//Its port will be assigned by us.
			int nextPort = node_list.size()+1;
			if(nextPort/10==0)
			{
				char newPort[5]; 
				sprintf(newPort,"1200%d",nextPort);
				nextPort = atoi(newPort);
			}
			else if(nextPort/100==0)
			{
				char newPort[5]; 
				sprintf(newPort,"120%d",nextPort);
				nextPort = atoi(newPort);
			}

			//insert it into nodelist
			Node *joinedNode = new Node(tokenlist[1],nextPort);
			node_list.push_back(joinedNode);
			sort(node_list.begin(),node_list.end(),compareNode);

			unsigned long long my_machine_id = joinedNode->getMachineId();
			tokenlist[0]=to_string(my_machine_id);
			tokenlist[1]=to_string(joinedNode->getPort());
			//Get the list position of that node
			int pos;
			for(int i=0;i<node_list.size();i++)
			{
				if(node_list[i]->getMachineId()==my_machine_id)
				{
					pos=i;
					break;
				}
			}
			Node *successor;
			Node *predecessor;
			//get successor
			if(pos!=node_list.size()-1)
			{
				//there exist a successor
				successor = node_list[pos+1];
				tokenlist[2]=to_string(successor->getMachineId());
				tokenlist[3]=to_string(successor->getPort());
			}
			else
			{
				successor=node_list[0];
				tokenlist[2]=to_string(successor->getMachineId());
				tokenlist[3]=to_string(successor->getPort());
			}

			if(pos!=0)
			{
				//there exist a predecessor
				predecessor = node_list[pos-1];
				tokenlist[4]=to_string(predecessor->getMachineId());
				tokenlist[5]=to_string(predecessor->getPort());
			}
			else
			{
				predecessor=node_list[node_list.size()-1];
				tokenlist[4]=to_string(predecessor->getMachineId());
				tokenlist[5]=to_string(predecessor->getPort());
			}

			//now send the information to the joining node
			string send = tokenlist[0];
			for(int i=1;i<6;i++)
			{
				send.append(":");
				send.append(tokenlist[i]);
			}
			cout<<"I will send "<<send<<endl;

			//send the information using the same socket.

		}


	}

}
