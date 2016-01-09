//
//  headers.h
//  Chord_1
//
//  Created by SABYASACHEE BARUAH on 03/04/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#ifndef Chord_1_headers_h
#define Chord_1_headers_h

#include <iostream>
#include <string>
#include <list>
#include <tuple>
#include <set>
#include <vector>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define M       3
#define ROOT    "/Users/Sabyasachee/"
#define KEY     99
#define start(n,i)  (n + (1<<(i)))%(1<<M)
#define S       1           // successor
#define C       2           // closest preceding finger
#define F       3           // findsuccessor
#define P       4           // predecessor
#define SP      5           // setpredecessor
#define U       6           // update fingertable
#define I       7           // transfer Index from user
#define A       8           // add user
#define R       9           // give back users set for given key
#define B       96          // remove user
#define V       97          // remove node
#define J       98          // transfering keys to successor
#define D       99          // download file request

#define Q       10          // simple reply
#define SQ      11          // response to S
#define CQ      12          // response to C
#define FQ      13          // response to F
#define PQ      14          // response to P

struct shared {
    int ports[1<<M];
    int available[1<<M];
    int nport;
    char nname[50];
};

class Address;
int hash(std::string);
int file_select(const struct direct*);
int create();
std::tuple<int, Address> getport_address(int, std::string);
void leave(Address, Address);
bool onrange(int, int, int); //[]
std::string comm(int);
#endif
