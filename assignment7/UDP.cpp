//
//  UDP.cpp
//  Chord_1
//
//  Created by SABYASACHEE BARUAH on 04/04/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "classes.h"

UDP::UDP(int port) {
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char myport[5];
    sprintf(myport, "%d", port);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, myport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((socket = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        std::cerr<<"listener: failed to bind socket\n";
        exit(2);
    }
    freeaddrinfo(servinfo);
}

void UDP::send(Packet p) {
    std::string message = p.serialize();
    char buf[200];
    strcpy(buf, message.c_str());
    Address destination = p.getdestination();
    int port = destination.getport();
    int sockfd = -1;
    struct addrinfo hints, *servinfo, *ptr;
    int rv;
    long numbytes;
    char myport[5];
    sprintf(myport, "%d", port);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo("127.0.0.1", myport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) {
        if ((sockfd = ::socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    
    if (ptr == NULL) {
        std::cerr<<"talker: failed to bind socket\n";
        exit(2);
    }
    if ((numbytes = sendto(socket, buf, strlen(buf), 0,
                           ptr->ai_addr, ptr->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    close(sockfd);
}

Packet UDP::receive() {
    char message[200];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    long numbytes;
    if ((numbytes = recvfrom(socket, message, 200, 0,
                             (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    message[numbytes] = '\0';
    Packet p(message);
    return p;
}

int UDP::getsocket() {return socket;}

UDP::~UDP(){
    close(socket);
}



