//
//  Connect.cpp
//  Chord_0
//
//  Created by SABYASACHEE BARUAH on 29/03/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "declarations.hxx"

#define HOST "127.0.0.1"

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int send_file(int sockfd, const char* filename){
    
    int fd = open(filename, O_RDONLY);
    char *line = (char*)malloc(80);
    long bytesread, bytestosend, bytessent, nbytes;
    
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

Connect::Connect() {}


Connect::Connect(int type, int port) throw (myExceptions){
    
    int rv, sockfd = 0, yes = 1;
    char myport[10];
    struct addrinfo hints, *servinfo, *p;
    myExceptions e;
    
    sprintf(myport, "%d", port);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;                            // set to AF_INET to force IPv4
    hints.ai_flags = AI_PASSIVE;                            // use my IP
    
    if (type == DATAGRAM) {
        hints.ai_socktype = SOCK_DGRAM;                     // use datagram socket
    }
    else {
        hints.ai_socktype = SOCK_STREAM;                    // use stream socket
    }

    if ((rv = getaddrinfo(NULL, myport, &hints, &servinfo)) != 0) {
        cerr<<"getaddrinfo: "<<gai_strerror(rv)<<endl;
        e.setmsg("Error in getting address structure list");
        throw e;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            
            perror("listener: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        e.setmsg("failed to bind socket");
        throw e;
    }
    
    freeaddrinfo(servinfo);
    
    if (type == STREAM) {
        if (listen(sockfd, 10) == -1) {
            perror("listen");
            e.setmsg("Error in listening");
            throw e;
        }
    }
    this->sockfd = sockfd;
}

void Connect::shut() {
    close(sockfd);
}

void Connect::setfile(string file) {
    this->file = file;
}

int Connect::getsocket() {
    return sockfd;
}


void Connect::send(Packet p) throw(myExceptions) {
    
    int port, rv, sockfd = 0;
    long numbytes;
    char myport[10];
    struct addrinfo hints, *servinfo, *ptr;
    address destination = p.getdestinaiton();
    myExceptions e;
    
    port = get<1>(destination);
    sprintf(myport, "%d", port);
    cout<<myport;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(HOST, myport, &hints, &servinfo)) != 0) {
        cerr<<"getaddrinfo: "<<gai_strerror(rv)<<endl;
        e.setmsg("Error in getting address structure list");
        throw e;
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
        e.setmsg("talker : failed to bind socket");
        throw e;
    }
    
    if ((numbytes = sendto(sockfd, &p, sizeof(p), 0,
                           ptr->ai_addr, ptr->ai_addrlen)) == -1) {
        perror("talker: sendto");
        e.setmsg("Error in sending packet");
        throw e;
    }
    
    freeaddrinfo(servinfo);
    
    cout<<"talker: sent "<<numbytes<<" bytes to "<<HOST<<endl;
    close(sockfd);
}

Packet Connect::recv() throw (myExceptions) {
    
    long numbytes;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr;
    Packet p;
    myExceptions e;
    
    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, &p, sizeof(p) , 0,
                             (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        e.setmsg("Error in receiving packet");
        throw e;
    }
    
    printf("listener: got packet from %s\n",inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
    cout<<"listener: packet is "<<numbytes<<" bytes long"<<endl;

    return p;
}

void Connect::sendfile(int newfd) throw (myExceptions) {
    
    myExceptions e;
    if(send_file(newfd, (char*)file.c_str()) == -1) {
        perror("send:");
        close(newfd);
        e.setmsg("Error in sending file");
    }
    close(newfd);
}

int Connect::connect(address server) throw (myExceptions) {
    
    struct addrinfo hints, *servinfo, *p;
    int rv, sockfd = 0;
    int port = get<1>(server);
    char myport[10];
    char s[INET6_ADDRSTRLEN];
    myExceptions e;
    
    sprintf(myport, "%d", port);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(HOST, myport, &hints, &servinfo)) != 0) {
        cerr<<"getaddrinfo: "<<gai_strerror(rv)<<endl;
        e.setmsg("connect error");
        throw e;
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        
        if (::connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        e.setmsg("client: failed to connect");
        throw e;
    }
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);
    
    freeaddrinfo(servinfo); // all done with this structure
    return sockfd;
}

void Connect::savefile(char *path, int sockfd) {
    
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);
    long numbytes;
    char buf[80];
    
    // reveive bytes from server as long as the connection is open
    while ((numbytes = ::recv(sockfd, buf, 80, 0)) != 0) {
        write(fd, buf, numbytes);
    }
    
    // close the file descriptor and socket
    close(fd);
    close(sockfd);
}









