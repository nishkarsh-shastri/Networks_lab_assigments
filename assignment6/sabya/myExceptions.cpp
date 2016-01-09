//
//  myExceptions.cpp
//  Chord_0
//
//  Created by SABYASACHEE BARUAH on 29/03/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "declarations.hxx"

void myExceptions::setmsg(string message) {
    this->message = message;
}

void myExceptions::printmsg() {
    cout<<message;
}
