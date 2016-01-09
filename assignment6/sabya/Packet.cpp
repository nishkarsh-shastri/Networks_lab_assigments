//
//  Packet.cpp
//  Chord_0
//
//  Created by SABYASACHEE BARUAH on 29/03/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "declarations.hxx"

// Cannot delete data
Packet::~Packet() {}

int Packet::getcmd() {return command;}
void* Packet::getdata() {return data;}
address Packet::getsource() {return source;}
address Packet::getdestinaiton() {return destination;}

void Packet::setcmd(int command) {this->command = command;}
void Packet::setdata(void *data) {this->data = data;}
void Packet::setsource(address source) {this->source = source;}
void Packet::setdestination(address destination) {this->destination = destination;}
