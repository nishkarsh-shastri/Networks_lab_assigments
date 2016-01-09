//
//  classes.cpp
//  Chord_1
//
//  Created by SABYASACHEE BARUAH on 03/04/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "classes.h"

Address::Address() {
    this->identifier = -1;
}

Address::Address(int port, std::string name) {
    this->port = port;
    this->name = name;
    this->identifier = hash(std::to_string(port));
}

Address::Address(std::string message) {
    if (message != "") {
        char* cstring = (char*)malloc(50), *tofree;
        strcpy(cstring, message.c_str());
        tofree = cstring;
        char* tokenlist[3];
        tokenlist[0] = strsep(&cstring, ",");
        tokenlist[1] = strsep(&cstring, ",");
        tokenlist[2] = strsep(&cstring, ",");
        if(tokenlist[0])port = atoi(tokenlist[0]);
        if(tokenlist[1])identifier = atoi(tokenlist[1]);
        if(tokenlist[2])name = tokenlist[2];
        free(tofree);
    }
    else port = -1;
}

int Address::getid() {
    return identifier;
}

int Address::getport() {
    return port;
}

std::string Address::getname() {
    return name;
}

std::string Address::serialize() {
    return std::to_string(port) + "," + std::to_string(identifier) + "," + name;
}

void Address::show() {
    std::cout<<"Address : port = "<<port<<", id = "<<identifier<<", name = "<<name<<std::endl;
}

bool Address::operator==(Address &a) {
    if (port == a.getport() && identifier == a.getid() && name == a.getname()) {
        return true;
    }
    return false;
}

Packet::Packet(Address source, Address destination, int command):source(source), destination(destination) {
    this->command = command;
}

Packet::Packet(Address source, Address destination, int command, int key):source(source), destination(destination) {
    this->command = command;
    this->key = key;
}

Packet::Packet(Address source, Address destination, int command, Address addr):source(source), destination(destination), data(addr) {
    this->command = command;
}

Packet::Packet(Address source, Address destination, int command, int key, Address addr):source(source), destination(destination), data(addr) {
    this->command = command;
    this->key = key;
}

Packet::Packet(Address source, Address destination, int command, std::string file):source(source), destination(destination), file(file) {
    this->command = command;
}

Packet::Packet(std::string message) {
    char *cstring = (char*)malloc(1024), *tofree;
    strcpy(cstring, message.c_str());
    tofree = cstring;
    char* tokenlist[6];
    tokenlist[0] = strsep(&cstring, ";");
    tokenlist[1] = strsep(&cstring, ";");
    tokenlist[2] = strsep(&cstring, ";");
    tokenlist[3] = strsep(&cstring, ";");
    tokenlist[4] = strsep(&cstring, ";");
    tokenlist[5] = strsep(&cstring, ";");
    source = Address(tokenlist[0]);
    destination = Address(tokenlist[1]);
    data = Address(tokenlist[4]);
    command = atoi(tokenlist[2]);
    key = atoi(tokenlist[3]);
    file = tokenlist[5];
    free(tofree);
}

std::string Packet::serialize() {
    return source.serialize() + ";" + destination.serialize() + ";" + std::to_string(command) + ";" +
    std::to_string(key) + ";" + data.serialize() + ";" + file;
}

void Packet::show() {
    std::cout<<"source : ";source.show();
    std::cout<<"destination : ";destination.show();
    std::cout<<"command : "<<comm(command)<<std::endl;
    std::cout<<"key : "<<key<<std::endl;
    std::cout<<"data : ";data.show();
    std::cout<<"file : "<<file<<std::endl;
}

Address Packet::getaddress() {return data;}
int Packet::getkey() {return key;}
std::string Packet::getfile() {return file;}
int Packet::getcommand() {return command;}
Address Packet::getdestination() {return destination;}
Address Packet::getsource() {return source;}
