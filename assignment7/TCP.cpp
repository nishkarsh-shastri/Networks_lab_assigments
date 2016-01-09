//
//  TCP.cpp
//  Chord_1
//
//  Created by SABYASACHEE BARUAH on 04/04/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "classes.h"

TCP::TCP(int port) {
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    char myport[5];
    int rv;
    sprintf(myport, "%d", port);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, myport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((socket = ::socket(p->ai_family, p->ai_socktype,
                               p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (::bind(socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL)  {
        std::cerr<<"server: failed to bind\n";
        exit(1);
    }
    freeaddrinfo(servinfo);
    if (listen(socket, 10) == -1) {
        perror("listen");
        exit(1);
    }
}

std::string TCP::receive() {
    char message[1024], fullmessage[1024];
    int newfd = accept();
    long numbytes;
    strcpy(fullmessage, "");
    while ((numbytes = recv(newfd, message, 1024, 0)) != 0) {
        message[numbytes] = '\0';
        strcat(fullmessage, message);
    }
    close(newfd);
    return message;
}

void TCP::send(std::string message, Address destination) {
    int port = destination.getport() + 1;
    int newfd = connect(port);
    char* cstring = (char*)message.c_str();
    long bytestosend = message.length(), bytessent = 0, nbytes;
    while (bytessent < bytestosend) {
        nbytes = ::send(newfd, cstring + bytessent, bytestosend - bytessent, 0);
        bytestosend -= nbytes;
        bytessent += nbytes;
    }
    close(newfd);
}

void TCP::receivefile(std::string filename) {
    int newfd = accept();
    long numbytes;
    char message[1024];
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    while ((numbytes = recv(newfd, message, 1024, 0)) != 0) {
        write(fd, message, numbytes);
    }
    close(fd);
    close(newfd);
}

void TCP::sendfile(std::string filename, Address destination) {
    int port = destination.getport() + 1;
    int fd = open(filename.c_str(), O_RDONLY);
    int newfd = connect(port);
    char *line = (char*)malloc(1024);
    long bytesread, bytestosend, bytessent, nbytes;
    while((bytesread = ::read(fd, (void*)line, 1024)) > 0) {
        bytestosend = bytesread;
        bytessent = 0;
        while(bytessent < bytestosend) {
            nbytes = ::send(newfd, line + bytessent, bytestosend - bytessent, 0);
            if(nbytes == -1) {
                close(fd);
                exit(1);
            }
            bytessent += nbytes;
        }
    }
    close(newfd);
    close(fd);
}

int TCP::accept() {
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int newfd = ::accept(socket, (struct sockaddr *)&their_addr, &sin_size);
    return newfd;
}

int TCP::connect(int port) {
    struct addrinfo hints, *servinfo, *p;
    int rv, sockfd = -1;
    char myport[5];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    sprintf(myport, "%d", port);
    if ((rv = getaddrinfo("127.0.0.1", myport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = ::socket(p->ai_family, p->ai_socktype,
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
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }
    freeaddrinfo(servinfo);
    return sockfd;
}

TCP::~TCP() {
    close(socket);
}




