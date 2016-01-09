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
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

using namespace std;


#define SERVERPORT "15555" 
#define NODEPORT "15556"   // the port users will be connecting to

#define MAXBUFLEN 100
#define MAXTOKENS 20

int myport;
int tcpport;

fileHash myHash[2000];
int numberOfHash=0;

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
    printf("The node server received %s\n",buf);
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

void store(string name,int x,int machineId,int Port)
{
    cout<<"Storing file "<<name<<" for machine id "<<machineId<<endl;
    myHash[numberOfHash].hash=x;
    myHash[numberOfHash].name=name;
    myHash[numberOfHash].source_port=Port;
    myHash[numberOfHash].source_machine_id=machineId;
    numberOfHash++;
    cout<<endl<<endl;
    for(int j=0;j<numberOfHash;j++)
    {
       cout<<myHash[j].hash<<" ||| " <<myHash[j].name<<" ||| "<<myHash[j].source_port<<" ||| "<<myHash[j].source_machine_id<<endl;
    }
}

void forward(Node* node,int source_machine_id,int source_machine_port,string fileName,int x)
{
    Command *sendcmd = new Command();
    cout<<"Forwarding file  "<<fileName<<" to "<<node->getSuccessor()->getMachineId()<<endl;
    sendcmd->com = "NEW_FILE";
    sendcmd->source_machine_id=source_machine_id;
    sendcmd->source_machine_port=source_machine_port;
    sendcmd->destination_machine_id=node->getSuccessor()->getMachineId();
    sendcmd->destination_machine_port=node->getSuccessor()->getPort();
    sendcmd->data = fileName;
    sendcmd->hash = x;
    string sen = sendcmd->com+":"+to_string(sendcmd->source_machine_port)+":"+to_string(sendcmd->source_machine_id)+":"+to_string(sendcmd->destination_machine_port)+":"+to_string(sendcmd->destination_machine_id)+":"+sendcmd->data+":"+to_string(sendcmd->hash);
    sendOverDgramSocket(sen,sendcmd->destination_machine_port);
}



void checkAndUpdate(Node *node,Command cmd)
{
    string fileName = cmd.data;
    int x = cmd.hash;
    if(node->getMachineId()>=x)
        {
            if(node->getMachineId()==x || node->getPredecessor()->getMachineId()<x)
            {
                //store the hash here
                store(fileName,x,cmd.source_machine_id,cmd.source_machine_port);
            }
            else if(node->getPredecessor()->getMachineId()>node->getMachineId())
            {
                //store the hash here
                store(fileName,x,cmd.source_machine_id,cmd.source_machine_port);
            }
            else
            {
                //forward to the successor
                cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                forward(node,cmd.source_machine_id,cmd.source_machine_port,fileName,x);
            }
        }
    else
        {
            if(node->getPredecessor()->getMachineId()>node->getMachineId())
            {
                if(node->getPredecessor()->getMachineId()<x)
                {
                    //store the hash here
                    store(fileName,x,cmd.source_machine_id,cmd.source_machine_port);
                }
                else
                {
                    cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                    forward(node,cmd.source_machine_id,cmd.source_machine_port,fileName,x);
                }
            }
            else
            {
                cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                //forward it to successor
                forward(node,cmd.source_machine_id,cmd.source_machine_port,fileName,x);
            }
        }
}


bool isFilePresent(Node* node,string dir,string name)
{
    DIR *directory;
    struct dirent *ent;
    string filesName[2000];
    int number=0;
    dir = dir+"/";
    if ((directory = opendir (dir.c_str())) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (directory)) != NULL) {
        printf ("%s\n", ent->d_name);
        string s = ent->d_name;
        if(s==name)
            return false;
      }
      closedir (directory);
    } 
    return true;
}
void sendFileTo(Node* node,string dir)
{
    DIR *directory;
    struct dirent *ent;
    string filesName[2000];
    int number=0;
    dir = dir+"/";
    if ((directory = opendir (dir.c_str())) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (directory)) != NULL) {
        printf ("%s\n", ent->d_name);
        string s = ent->d_name;
        if(s!="."&&s!="..")
        {
            filesName[number++]=s;
        }
      }
      closedir (directory);
    } 
    else {
      /* could not open directory */
      perror ("Could not find directory");
      // return EXIT_FAILURE;
    }

    //send these files to get hashed
    for(int i=0;i<number;i++)
    {
        int x = oat_hash(filesName[i].c_str(),strlen(filesName[i].c_str()))%8;
        if(node->getMachineId()>=x)
        {
            if(node->getMachineId()==x || node->getPredecessor()->getMachineId()<x)
            {
                //store the hash here
                store(filesName[i],x,node->getMachineId(),node->getPort());
            }
            else if(node->getPredecessor()->getMachineId()>node->getMachineId())
            {
                //store the hash here
                store(filesName[i],x,node->getMachineId(),node->getPort());
            }
            else
            {
                //forward to the successor

                cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                forward(node,node->getMachineId(),node->getPort(),filesName[i],x);
            }
        }
        else
        {
            if(node->getPredecessor()->getMachineId()>node->getMachineId())
            {
                if(node->getPredecessor()->getMachineId()<x)
                {
                    //store the hash here
                    store(filesName[i],x,node->getMachineId(),node->getPort());
                }
                else
                {   
                    //forward it to successor
                     cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                    forward(node,node->getMachineId(),node->getPort(),filesName[i],x);
                }
            }
            else
            {
                //forward it to successor
                cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                forward(node,node->getMachineId(),node->getPort(),filesName[i],x);
            }
        }
    }
}

void searchFurtherCommand(Node *node,int source_machine_id,int source_machine_port,string fileName,int x)
{
    Command *sendcmd = new Command();
    cout<<"Forwarding file search "<<fileName<<" to "<<node->getSuccessor()->getMachineId()<<endl;
    sendcmd->com = "FILE_SEARCH";
    sendcmd->source_machine_id=source_machine_id;
    sendcmd->source_machine_port=source_machine_port;
    sendcmd->destination_machine_id=node->getSuccessor()->getMachineId();
    sendcmd->destination_machine_port=node->getSuccessor()->getPort();
    sendcmd->data = fileName;
    sendcmd->hash = x;
    string sen = sendcmd->com+":"+to_string(sendcmd->source_machine_port)+":"+to_string(sendcmd->source_machine_id)+":"+to_string(sendcmd->destination_machine_port)+":"+to_string(sendcmd->destination_machine_id)+":"+sendcmd->data+":"+to_string(sendcmd->hash);
    sendOverDgramSocket(sen,sendcmd->destination_machine_port);
    sleep(1);
}

void sendFindCommand(int source_machine_id,int source_machine_port,int target_machine_id,int target_machine_port,string fileName)
{
    Command *sendcmd = new Command();
    cout<<"Found file "<<fileName<<" and reporting to "<<target_machine_id<<endl;
    sendcmd->com = "FILE_FOUND";
    sendcmd->source_machine_id=source_machine_id;
    sendcmd->source_machine_port=source_machine_port;
    sendcmd->destination_machine_id=target_machine_id;
    sendcmd->destination_machine_port=target_machine_port;
    sendcmd->data = fileName;
    sendcmd->hash = 0;
    string sen = sendcmd->com+":"+to_string(sendcmd->source_machine_port)+":"+to_string(sendcmd->source_machine_id)+":"+to_string(sendcmd->destination_machine_port)+":"+to_string(sendcmd->destination_machine_id)+":"+sendcmd->data+":"+to_string(sendcmd->hash);
    sendOverDgramSocket(sen,sendcmd->destination_machine_port);
    sleep(1);
}


void searchAmongFiles(Node *node,Command cmd)
{
    string name = cmd.data;
    int x = cmd.hash;
    fileHash tempHash;
    int flag = 0;
    if(node->getMachineId()>=x)
    {
        if(node->getMachineId()==x || node->getPredecessor()->getMachineId()<x)
        {
            int j;
            for(j=0;j<numberOfHash;j++)
            {
                if (myHash[j].name == name)
                {
                    cout<<"File found at "<<node->getMachineId()<<endl;
                    tempHash = myHash[j];
                    flag = 1;
                    break;
                }
            }
            if(j==numberOfHash)
            {
                cout<<"InvalidFileName"<<endl;
            }

        }
        else if(node->getPredecessor()->getMachineId()>node->getMachineId())
        {
            //store the hash here
            //store(fileName,x,cmd.source_machine_id,cmd.source_machine_port);
            int j;
            for(j=0;j<numberOfHash;j++)
            {
                if (myHash[j].name == name)
                {
                    cout<<"File found at "<<node->getMachineId()<<endl;
                    tempHash = myHash[j];
                    flag = 1;
                    break;
                }
            }
            if(j==numberOfHash)
            {
                cout<<"InvalidFileName"<<endl;
            }
        }
        else
        {
            //forward to the successor
            cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
            searchFurtherCommand(node,cmd.source_machine_id,cmd.source_machine_port,name,x);
        }
    }
    else
    {
        if(node->getPredecessor()->getMachineId()>node->getMachineId())
        {
            if(node->getPredecessor()->getMachineId()<x)
            {
                //store the hash here
                //store(fileName,x,cmd.source_machine_id,cmd.source_machine_port);
                int j;
                for(j=0;j<numberOfHash;j++)
                {
                    if (myHash[j].name == name)
                    {
                        cout<<"File found at "<<node->getMachineId()<<endl;
                        tempHash = myHash[j];
                        flag = 1;
                        break;
                    }
                }
                if(j==numberOfHash)
                {
                    cout<<"InvalidFileName"<<endl;
                }
            }
            else
            {
                cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                searchFurtherCommand(node,cmd.source_machine_id,cmd.source_machine_port,name,x);
            }
        }
        else
        {
            cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
            //forward it to successor
            searchFurtherCommand(node,cmd.source_machine_id,cmd.source_machine_port,name,x);
        }
    }
    if(flag==1)
    {
        cout<<"Sending the port of the file destination to "<<tempHash.source_machine_id<<" and port "<<tempHash.source_port<<endl;
        sendFindCommand(tempHash.source_machine_id,tempHash.source_port,cmd.source_machine_id,cmd.source_machine_port,name);
    }
}

void searchAndDownload(Node *node,string name)
{
    int x = oat_hash(name.c_str(),strlen(name.c_str()))%8;
    int flag = 0;
    fileHash tempHash;
    if(node->getMachineId()>=x)
        {
            if(node->getMachineId()==x || node->getPredecessor()->getMachineId()<x)
            {
                //store the hash here
                //store(fileName,x,cmd.source_machine_id,cmd.source_machine_port);
                //file exist in this node itself
                int j;
                for(j=0;j<numberOfHash;j++)
                {
                    if (myHash[j].name == name)
                    {
                        cout<<"File already present in my hashes"<<endl;
                        tempHash = myHash[j];
                        flag=1;
                        break;
                    }
                }
                if(j==numberOfHash)
                {
                    cout<<"InvalidFileName"<<endl;
                }

            }
            else if(node->getPredecessor()->getMachineId()>node->getMachineId())
            {
                //store the hash here
                //store(fileName,x,cmd.source_machine_id,cmd.source_machine_port);
                int j;
                for(j=0;j<numberOfHash;j++)
                {
                    if (myHash[j].name == name)
                    {
                        cout<<"File already present in my hashes"<<endl;
                        tempHash = myHash[j];
                        flag=1;
                        break;
                    }
                }
                if(j==numberOfHash)
                {
                    cout<<"InvalidFileName"<<endl;
                }
            }
            else
            {
                //forward to the successor
                cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                searchFurtherCommand(node,node->getMachineId(),node->getPort(),name,x);
            }
        }
    else
        {
            if(node->getPredecessor()->getMachineId()>node->getMachineId())
            {
                if(node->getPredecessor()->getMachineId()<x)
                {
                    //store the hash here
                    int j;
                    for(j=0;j<numberOfHash;j++)
                    {
                        if (myHash[j].name == name)
                        {
                            cout<<"File already present in my hash directory"<<endl;
                            tempHash = myHash[j];
                            flag=1;
                            break;
                        }
                    }
                    if(j==numberOfHash)
                    {
                        cout<<"InvalidFileName"<<endl;
                    }
                }
                else
                {
                    cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                    searchFurtherCommand(node,node->getMachineId(),node->getPort(),name,x);
                }
            }
            else if(flag==0)
            {
                cout<<"Forwarding it to successor "<<node->getSuccessor()->getMachineId()<<endl;
                //forward it to successor
                searchFurtherCommand(node,node->getMachineId(),node->getPort(),name,x);
            }
           
        }
        if(flag==1)
        {
            cout<<"Sending the port of the destination of "<<tempHash.source_machine_id<<" to "<<tempHash.source_port<<endl;
            sendFindCommand(tempHash.source_machine_id,tempHash.source_port,node->getMachineId(),node->getPort(),name);
        }
}

void downloadFileOverTcp(int port_no,string filename,string download)
{
    int port, rv, sockfd = 0;
    long numbytes;
    char myport[10];
    struct addrinfo hints, *servinfo, *ptr;
    char s[INET6_ADDRSTRLEN];
    port = port_no;
    sprintf(myport, "%d", port);
    cout<<myport<<endl;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int fd;
    
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
        if (connect(sockfd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
                close(sockfd);
                perror("client: connect");
                continue;
        }
        
        break;
    }
    
    if (ptr == NULL) {
        perror("Failed to bind tcp socket:");
        exit(1);
    }
    char buf[80];
    inet_ntop(ptr->ai_family, get_in_addr((struct sockaddr *)ptr->ai_addr),s, sizeof s);
    printf("client: connecting to %s\n", s);
        
    freeaddrinfo(servinfo);   
    cout<<filename<<endl;
    cout<<"Requested "<<filename.c_str()<<endl;
    if(send(sockfd, filename.c_str(), strlen(filename.c_str()), 0) == -1) {
            perror("client : send");
            exit(1);
        }
    download = download+"/"+filename;
    fd = open(download.c_str(), O_WRONLY | O_CREAT | O_TRUNC,0666);
    while ((numbytes = recv(sockfd, buf, 80, 0)) != 0) {
            write(fd, buf, numbytes);
        }

    close(sockfd);
    printf("File downloaded successfully...\n");
}

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

int main(int argc, char *argv[])
{
    int sockfd,sockfd2,sockfd_succ,sockfd_pred;
    struct addrinfo hints,hints2, *servinfo, *p,*q;
    int rv, numbytes, yes = 1, len = 0, indexsize = 10, i = 0,j;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN], s[INET6_ADDRSTRLEN], *line = NULL, *tofree, *token, *reply = NULL;
    socklen_t addr_len;    
    pid_t childpid;
    int new_fd;

    if (argc != 4) {
        fprintf(stderr,"usage: hostname sharedfolderName downloadfolderName\n");
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
    printf("The node server received %s\n",buf);
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
        string sen = sendcmd->com+":"+to_string(sendcmd->source_machine_port)+":"+to_string(sendcmd->source_machine_id)+":"+to_string(sendcmd->destination_machine_port)+":"+to_string(sendcmd->destination_machine_id)+":"+sendcmd->data+":"+to_string(sendcmd->hash);
        strcpy(buf,sen.c_str());
        sendOverDgramSocket(sen,sendcmd->destination_machine_port);
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
        string sen = sendcmd->com+":"+to_string(sendcmd->source_machine_port)+":"+to_string(sendcmd->source_machine_id)+":"+to_string(sendcmd->destination_machine_port)+":"+to_string(sendcmd->destination_machine_id)+":"+sendcmd->data+":"+to_string(sendcmd->hash);
        sendOverDgramSocket(sen,sendcmd->destination_machine_port);
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
                        new_fd = accept(tcpSocket,
                            (struct sockaddr *)&remoteaddr,
                            &addrlen);

                        if (newfd == -1) 
                        {
                            perror("accept");
                        } 
                        else 
                        {
                            printf("selectserver: new connection from %s on "
                                "socket %d\n",
                                inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN),newfd);
                            if ((childpid = fork()) == 0) 
                            {
                                close(sockfd);
                                // receive filename for downloading
                                if ((nbytes =recv(new_fd, buf,80, 0)) == -1) {
                                    perror("Error : filename receive error...");
                                    exit(1);
                                }

                                // get full pathname
                                //printf("%s\n", filename);
                                buf[nbytes]='\0';
                                cout<<"Requested :: "<<buf<<endl;
                                string filename = buf;
                                string dnld = argv[2];
                                dnld=dnld+"/"+filename;
                                cout<<"downloading form "<<dnld<<endl;
                                // send file to client
                                if(send_file(new_fd, dnld.c_str()) == -1) {
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
                        else if(cmd.com=="NEW_FILE")
                        {
                            checkAndUpdate(myNode,cmd);
                            cout<<endl<<endl;
                            for(int j=0;j<numberOfHash;j++)
                            {
                               cout<<myHash[j].hash<<" ||| "<<myHash[j].name<<" ||| "<<myHash[j].source_port<<" ||| "<<myHash[j].source_machine_id<<endl;
                            }
                        }
                        else if(cmd.com=="FILE_SEARCH")
                        {
                            searchAmongFiles(myNode,cmd);
                        }
                        else if(cmd.com=="FILE_FOUND")
                        {
                            cout<<"FOUND THE REQUESTED FILE"<<endl;
                            cout<<"It is at "<<cmd.source_machine_id<<" and whose port is "<<cmd.source_machine_port<<endl;
                            cout<<"I am going to download that file "<<endl;
                            //we will download it over tcp socket
                            downloadFileOverTcp(cmd.source_machine_port+3000,cmd.data,argv[3]);
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
                            }

                            string readCom = buf;
                            if(readCom=="file")
                            {
                                sendFileTo(myNode,argv[2]);
                            }
                            else if(readCom=="list")
                            {
                                for(int j=0;j<numberOfHash;j++)
                                {
                                   cout<<myHash[j].hash<<" ||| "<<myHash[j].name<<" ||| "<<myHash[j].source_port<<" ||| "<<myHash[j].source_machine_id<<endl;
                                }   
                            }
                            else
                            {
                                if(isFilePresent(myNode,argv[2],readCom))
                                    searchAndDownload(myNode,readCom);
                                else
                                    printf("File already in downloads folder\n");
                            }

                    }// END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    }
    close(tcpSocket);
    close(dgramsocket);
}