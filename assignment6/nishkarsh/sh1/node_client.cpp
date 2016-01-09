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
#include <algorithm>
#include <time.h>
#include <dirent.h>

using namespace std;


#define SERVERPORT "15555" 
#define NODEPORT "15556"   // the port users will be connecting to

#define MAXBUFLEN 100
#define MAXTOKENS 20

int myport;
int tcpport;



void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


Command getCommand(char *buf)
{
    int numberOfTokens=0;
    string tokenlist[MAXTOKENS];
    char *pch;
    //I am assuming buf to be the string reply from client
    printf("The nodoe server received %s\n",buf);
    string rec;
    rec = strdup(buf);

    string node_ip;
    
    pch = strtok(buf,":");
    
    while(pch!=NULL)
    {
        tokenlist[numberOfTokens++]=strdup(pch);
        pch = strtok(NULL,":");
    }
    Command cmd;
    cmd.com=tokenlist[0];
    cmd.source_machine_port=atoi(tokenlist[1].c_str());
    cmd.source_machine_id=atoi(tokenlist[2].c_str());
    cmd.destination_machine_port=atoi(tokenlist[3].c_str());
    cmd.destination_machine_id=atoi(tokenlist[4].c_str());
    cmd.data = tokenlist[5];
    cmd.hash = atoi(tokenlist[6].c_str());
    return cmd;


}


void sendOverDgramSocket(string s,int my_port)
{
    
    char buf[MAXBUFLEN];
    strcpy(buf,s.c_str());
    int port, rv, sockfd = 0;
    long numbytes;
    char myport[10];
    struct addrinfo hints, *servinfo, *ptr;
    
    port = my_port;
    sprintf(myport, "%d", port);
    cout<<myport;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    
    if ((rv = getaddrinfo(NULL, myport, &hints, &servinfo)) != 0) {
        cerr<<"getaddrinfo: "<<gai_strerror(rv)<<endl;
        perror("getaddrinfo:");
    }
    
    for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) {
        if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype,
                             ptr->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        
        break;
    }
    
    if (ptr == NULL) {
        perror("Failed to bind socket:");
        exit(1);
    }

    
    if ((numbytes = sendto(sockfd, &buf, sizeof(buf), 0,
                           ptr->ai_addr, ptr->ai_addrlen)) == -1) {
        perror("talker: sendto");
        close(sockfd);
        exit(1);
    }
    
    freeaddrinfo(servinfo);    
    close(sockfd);
}


void sendFileTo(Node* node,string dir)
{
    DIR *directory;
    struct dirent *ent;
    dir = dir+"/";
    if ((directory = opendir (dir.c_str())) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (directory)) != NULL) {
        printf ("%s\n", ent->d_name);
      }
      closedir (directory);
    } 
    else {
      /* could not open directory */
      perror ("Could not find directory");
      // return EXIT_FAILURE;
    }
}

int main(int argc, char *argv[])
{
    int sockfd,sockfd2,sockfd_succ,sockfd_pred;
    struct addrinfo hints,hints2, *servinfo, *p,*q;
    int rv, numbytes, yes = 1, len = 0, indexsize = 10, i = 0,j;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN], s[INET6_ADDRSTRLEN], *line = NULL, *tofree, *token, *reply = NULL;
    socklen_t addr_len;    

    if (argc != 3) {
        fprintf(stderr,"usage: hostname folderName\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    if ((rv = getaddrinfo(NULL,NODEPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(q = servinfo; q != NULL; q = q->ai_next) {
        if ((sockfd2 = socket(p->ai_family, q->ai_socktype,
                q->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd2, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd2, q->ai_addr, q->ai_addrlen) == -1) {
            close(sockfd2);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (q == NULL) {
        fprintf(stderr, "client: failed to bind as nodeserver\n");
        return 2;
    }

    if ((numbytes = sendto(sockfd,"NEW", strlen("NEW"), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    //sendOverDgramSocket("NEW",15555);


    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);

    printf("node server: waiting to recvfrom...\n"); 
        //Here I am assuming that central server is waiting to get a string starting with NEW
    addr_len = sizeof(their_addr);
    if((numbytes=recvfrom(sockfd2,buf,MAXBUFLEN-1,0,(struct sockaddr*)&their_addr,&addr_len))==-1)
    {
        perror("recvfrom");
        exit(1);
    }
    printf("node got packet from %s\n",inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr*)&their_addr),s,sizeof s));
    printf("node packet is %d bytes long \n",numbytes);
    buf[numbytes]='\0';

    int numberOfTokens=0;
    string tokenlist[MAXTOKENS];
    char *pch;
    //I am assuming buf to be the string reply from client
    printf("The nodoe server received %s\n",buf);
    string rec;
    rec = strdup(buf);

    string node_ip;
    
    pch = strtok(buf,":");
    
    while(pch!=NULL)
    {
        tokenlist[numberOfTokens++]=strdup(pch);
        pch = strtok(NULL,":");
    }

    //set up the nodes for the present client
    Node *myNode = new Node("127.0.0.1",atoi(tokenlist[1].c_str()));
    myNode->setSuccessor(new Node("127.0.0.1",atoi(tokenlist[3].c_str())));
    myNode->setPredecessor(new Node("127.0.0.1",atoi(tokenlist[5].c_str())));

    cout<<"My MachineId :: "<<myNode->getMachineId()<<endl;
    cout<<"successor MachineId :: "<<myNode->getSuccessor()->getMachineId()<<endl;
    cout<<"predecessor MachineId :: "<<myNode->getPredecessor()->getMachineId()<<endl;


    close(sockfd);
    close(sockfd2);
    myport = myNode->getPort();
    tcpport = myNode->getPort()+3000;
    //create three sockets 
    // First one would be the client port for accepting UDP requests from other nodes
    // Second one would be the client port for accepting TCP requests from other nodes
    // Third one would be STDIN port for sending file requests to other nodes

    int dgramsocket,tcpSocket;

    string myport = to_string(myNode->getPort());


    memset(&hints2, 0, sizeof hints);
    hints2.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints2.ai_socktype = SOCK_DGRAM;
    hints2.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL,myport.c_str(), &hints2, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(q = servinfo; q != NULL; q = q->ai_next) {
        if ((dgramsocket = socket(p->ai_family, q->ai_socktype,
                q->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(dgramsocket, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(dgramsocket, q->ai_addr, q->ai_addrlen) == -1) {
            close(dgramsocket);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (q == NULL) {
        fprintf(stderr, "client: failed to bind as nodeserver\n");
        return 2;
    }   

    struct addrinfo *ai;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL,to_string(tcpport).c_str(), &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        tcpSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (tcpSocket < 0) { 
            continue;
        }       
        // lose the pesky "address already in use" error message
        setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(tcpSocket, p->ai_addr, p->ai_addrlen) < 0) {
            close(tcpSocket);
            continue;
        }

        break;
    }
    if (p == NULL) {
        fprintf(stderr, "selectserver tcpSocket: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai);

    if(listen(tcpSocket,10)==-1)
    {
        perror("tcpSocket");
        exit(3);
    }  


    if(myNode->getMachineId()!=myNode->getSuccessor()->getMachineId())
    {
        Command *sendcmd = new Command();
        sendcmd->com = "NEW_PREDECESSOR";
        sendcmd->source_machine_port=myNode->getPort();
        sendcmd->source_machine_id=myNode->getMachineId();
        sendcmd->destination_machine_id=myNode->getSuccessor()->getMachineId();
        sendcmd->destination_machine_port=myNode->getSuccessor()->getPort();
        sendcmd->data = "NODATA";
        sendcmd->hash = -1;
        string sen = sendcmd->com+":"+to_string(sendcmd->source_machine_port)+":"+to_string(sendcmd->source_machine_id)+":"+to_string(sendcmd->destination_machine_port)+":"+to_string(sendcmd->destination_machine_id)+":"+sendcmd->data+to_string(sendcmd->hash);
        strcpy(buf,sen.c_str());
        sendOverDgramSocket(sen,sendcmd->destination_machine_port);
        sleep(2);
    }

    if(myNode->getMachineId()!=myNode->getPredecessor()->getMachineId())
    {
        Command *sendcmd = new Command();
        sendcmd->com = "NEW_SUCCESSOR";
        sendcmd->source_machine_port=myNode->getPort();
        sendcmd->source_machine_id=myNode->getMachineId();
        sendcmd->destination_machine_id=myNode->getPredecessor()->getMachineId();
        sendcmd->destination_machine_port=myNode->getPredecessor()->getPort();
        sendcmd->data = "NODATA";
        sendcmd->hash = -1;
        string sen = sendcmd->com+":"+to_string(sendcmd->source_machine_port)+":"+to_string(sendcmd->source_machine_id)+":"+to_string(sendcmd->destination_machine_port)+":"+to_string(sendcmd->destination_machine_id)+":"+sendcmd->data+to_string(sendcmd->hash);
        sendOverDgramSocket(sen,sendcmd->destination_machine_port);
        sleep(2);
    }

    fd_set master;
    fd_set read_fds;
    socklen_t addrlen;
    int nbytes;
    int fdmax;
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    char remoteIP[INET6_ADDRSTRLEN];
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    FD_SET(dgramsocket,&master);    
    FD_SET(tcpSocket,&master);
    FD_SET(0,&master);
    fdmax = max(dgramsocket,tcpSocket);

    for(;;)
    {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds)) 
            { // we got one!!
                
                    if (i == tcpSocket) 
                    {
                        // handle file sharing connections this will be done at last
                        addrlen = sizeof remoteaddr;
                        newfd = accept(tcpSocket,
                            (struct sockaddr *)&remoteaddr,
                            &addrlen);

                        if (newfd == -1) 
                        {
                            perror("accept");
                        } 
                        else 
                        {
                            FD_SET(newfd, &master); // add to master set
                            if (newfd > fdmax) 
                            {    // keep track of the max
                                fdmax = newfd;
                            }
                            printf("selectserver: new connection from %s on "
                                "socket %d\n",
                                inet_ntop(remoteaddr.ss_family,
                                    get_in_addr((struct sockaddr*)&remoteaddr),
                                    remoteIP, INET6_ADDRSTRLEN),
                                newfd);
                        }
                    } 
                    else if(i==dgramsocket)//handle dgram connections from other nodes
                    {
                        // handle data from a client
                            //Here I am assuming that central server is waiting to get a string starting with NEW
                        addr_len = sizeof(their_addr);

                        if((numbytes=recvfrom(dgramsocket,buf,MAXBUFLEN-1,0,(struct sockaddr*)&their_addr,&addr_len))==-1)
                        {
                            perror("recvfrom");
                            exit(1);
                        }
                        buf[numbytes]='\0';

                        Command cmd = getCommand(buf);

                        if(cmd.com=="NEW_PREDECESSOR")
                        {
                            cout<<"New predecessor with MachineId  "<<cmd.source_machine_id<<endl;
                            cout<<"New predecessor with MachinePort  "<<cmd.source_machine_port<<endl;
                            myNode->setPredecessor(new Node("127.0.0.1",cmd.source_machine_port));
                            myNode->getStatus();
                        }
                        else if(cmd.com=="NEW_SUCCESSOR")
                        {
                            cout<<"New successor with MachineId  "<<cmd.source_machine_id<<endl;
                            cout<<"New successor with MachinePort  "<<cmd.source_machine_port<<endl;
                            myNode->setSuccessor(new Node("127.0.0.1",cmd.source_machine_port));
                            myNode->getStatus();
                        }


                    }
                    else if(i==0) 
                    {
                            if ((nbytes = read(i, buf, sizeof buf))<=0) 
                            {
                                // got error or connection closed by client
                                if (nbytes == 0) 
                                {
                                    // connection closed
                                    printf("selectserver: socket %d hung up\n", i);
                                } 
                                else 
                                {
                                    perror("recv stdin");
                                }
                                close(i); // bye!
                                FD_CLR(i, &master); // remove from master set
                            }
                            else
                            {
                                buf[nbytes-1]='\0';
                                cout<<"received from stdin::"<<buf<<endl;
                                cout<<"value"<<endl;
                            }
                            string readCom = buf;
                            if(readCom=="file")
                            {
                                sendFileTo(myNode,argv[2]);
                            }

                    }// END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    }
    close(tcpSocket);
    close(dgramsocket);
}