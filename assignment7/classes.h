//
//  classes.h
//  Chord_1
//
//  Created by SABYASACHEE BARUAH on 03/04/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#ifndef Chord_1_classes_h
#define Chord_1_classes_h
#include "headers.h"

class Address {
public:
    Address();
    Address(int, std::string);
    Address(std::string);
    void show();
    int getport();
    int getid();
    std::string getname();
    std::string serialize();
    bool operator==(Address&);
    //void operator=(Address&);
private:
    int port;
    int identifier;
    std::string name;
};

class Packet {
public:
    Packet(Address, Address, int);                      
    Packet(Address, Address, int, int);
    Packet(Address, Address, int, Address);
    Packet(Address, Address, int, int, Address);
    Packet(Address, Address, int, std::string);
    Packet(std::string);
    std::string serialize();
    Address getdestination();
    Address getsource();
    Address getaddress();
    int getkey();
    int getcommand();
    std::string getfile();
    void show();
private:
    Address source;
    Address destination;
    int key;
    Address data;
    std::string file;
    int command;
};

class UDP {
public:
    UDP(int);
    Packet receive();
    void send(Packet);
    int getsocket();
    ~UDP();
private:
    int socket;
};

class TCP {
public:
    TCP(int);
    std::string receive();
    void send(std::string, Address);
    void receivefile(std::string);
    void sendfile(std::string, Address);
    ~TCP();
private:
    int connect(int);
    int accept();
    int socket;
};

class Node {
public:
    Node(Address, UDP&, TCP&);
    void join(Address);
    void process();
    void download(std::string);
    void leave();
    void show();
private:
    Address findsuccessor(int);
    Address findpredecessor(int);
    Address closest_preceding_finger(int);
    void init_fingertable(Address);
    void updateothers();
    void update_fingertable(Address, int);
    void remove_node(Address, int, Address);
    
    Address me;
    Address predecessor;
    std::vector<Address> finger;
    std::list<std::pair<int, std::list<Address>>> index;
    std::set<std::string> filelist;
    std::string root;
    UDP& udp;
    TCP& tcp;
};

#endif
