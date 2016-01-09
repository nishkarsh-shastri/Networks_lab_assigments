//
//  declarations.h
//  Chord_0
//
//  Created by SABYASACHEE BARUAH on 28/03/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#ifndef Chord_0_declarations_h
#define Chord_0_declarations_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <dirent.h>

#include <iostream>
#include <fstream>
#include <tuple>
#include <string>
#include <list>
#include <vector>
#include <set>

#define GETSUCC     0                                                   // get successor
#define FOUNDSUCC   1                                                   // found successor
#define GETPRED     2                                                   // get predecessor
#define SENDPRED    3                                                   // send predecessor
#define SETSUCC     4                                                   // set successor
#define SETPRED     5                                                   // set predecessor
#define GETINDEX    6                                                   // get index
#define SENDINDEX   7                                                   // give index
#define REQUSERS    8                                                   // request user
#define SENDUSERS   9                                                   // send user
#define DOWNREQ     10                                                  // download request
#define SENDFILE    11                                                  // send file
#define SENDINFO    12                                                  // send new user info

#define DATAGRAM    0                                                   // datagram connection
#define STREAM      1                                                   // stream connection
#define NEW         0                                                   // joining state
#define ACTIVE      1                                                   // downloading state
#define LEAVE       2                                                   // leaving the network

using namespace std;

typedef tuple<int, int, string> address;                                // <ID, port, loginname> of node

struct data {
    int portlist[10][2];
    char name[20];
    int port;
    /*vector<pair<int, bool>> portlist;
    address n;*/
};

class myExceptions {
public:
    void printmsg();
    void setmsg(string);
private:
    string message;
};

class Packet {
public:
    ~Packet();
    
    int getcmd();
    address getsource();
    address getdestinaiton();
    void* getdata();
    
    void setcmd(int);
    void setsource(address);
    void setdestination(address);
    void setdata(void*);
    
private:
    address source;
    address destination;
    int command;
    void* data;
};

class Connect {
private:
    int sockfd;
    string file;
public:
    Connect();
    Connect(int, int) throw (myExceptions);                               // type = DATAGRAM or STREAM port
    void shut();
    
    int getsocket();
    void setfile(string);
    
    void send(Packet) throw (myExceptions);
    Packet recv() throw (myExceptions);
    int connect(address) throw (myExceptions);
    void sendfile(int) throw (myExceptions);
    void savefile(char*, int);
};

class Node {
public:
    Node();
    Node(string) throw (myExceptions);
    void join() throw (myExceptions);
    void leave() throw (myExceptions);
    void show();
    void process_datagram(Packet) throw (myExceptions);
    void process_stream(int) throw (myExceptions);
    void process_user(string, address, bool) throw (myExceptions);
    int getstreamsocket();
    int getdatagramsocket();
    
protected:
    void getsuccessor(Packet) throw (myExceptions);
    void foundsuccessor(Packet) throw (myExceptions);
    void getpredecessor(Packet) throw (myExceptions);
    //void sendpredecessor(Packet) throw (myExceptions);
    void setsuccessor(Packet);
    void setpredecessor(Packet);
    void requestusers(Packet) throw (myExceptions);
    void downloadrequest(Packet);
    void getindex(Packet) throw (myExceptions);
    void sendusers(Packet);
    void sendinfo(Packet);
    //void sendfile(Packet);
    
private:
    int state;
    address node;
    address successor;
    address predecessor;
    address enode;
    Connect datagram;
    Connect stream;
    
    list<string> filelist;
    vector<pair<int, set<address>> > index;
};

int getmax(int, int);
int dohash(int);
int dohash(string);
void *get_in_addr(struct sockaddr*);
int send_file(int, const char*);
#endif
