//
//  main.cpp
//  Chord_0
//
//  Created by SABYASACHEE BARUAH on 28/03/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "declarations.hxx"

int getmax(int a, int b) {
    if (a > b) {
        return a;
    }
    else return b;
}

int main(int argc, const char * argv[]) {
    
    fd_set set, masterset;
    int listener_stream, listener_datagram, fd_max, newfd, inputport, identifier;
    long numbytes;
    bool fileprompt = true;
    string name, userinput, fileinput;
    socklen_t addr_len;
    address user;
    Node n;
    Packet p;
    struct sockaddr_storage their_addr, remoteaddr;
    
    
    cout<<"Enter a loginname : ";
    cin>>name;
    
    try {
        n = Node(name);
    } catch (myExceptions e) {
        e.printmsg();
        return 1;
    }
    n.show();
    
    try {
        n.join();
    }
    catch(myExceptions e) {
        e.printmsg();
        return 1;
    }
    
    cout<<"Successfully joined P2P network..."<<endl;
    n.show();
    listener_datagram = n.getdatagramsocket();
    listener_stream = n.getstreamsocket();
    
    FD_ZERO(&masterset);
    FD_ZERO(&set);
    FD_SET(listener_stream, &masterset);
    FD_SET(listener_datagram, &masterset);
    FD_SET(STDIN_FILENO, &masterset);
    fd_max = getmax(listener_datagram, listener_stream) + 1;
    
    set = masterset;
    do {
        cout<<"Enter file (Press L to leave) : ";
        if (select(fd_max+1, &set, NULL, NULL, NULL) == -1) {
            perror("select");
            return 1;
        }
        cout<<"outside select\n";
        if (FD_ISSET(STDIN_FILENO, &set)) {
            cout<<"stdin :\n";
            if (fileprompt) {
                cin>>fileinput;
                if (fileinput == "L") {
                    break;
                }
            }
            else {
                cin>>inputport;
                cin>>userinput;
                identifier = dohash(inputport);
                user = address(identifier, inputport, userinput);
            }
            
            try {
                if (fileprompt) {
                    n.process_user(fileinput, user, fileprompt);
                }
                else {
                    n.process_user(fileinput, user, fileprompt);
                }
                
            } catch (myExceptions e) {
                e.printmsg();
                return 1;
            }
            
            fileprompt = !fileprompt;
        }
        
        if (FD_ISSET(listener_datagram, &set)) {
            cout<<"UDP :\n";
            addr_len = sizeof(their_addr);
            if ((numbytes = recvfrom(listener_datagram, &p, sizeof(p), 0, (struct sockaddr*)&their_addr, &addr_len)) == -1) {
                perror("recvfrom");
                return 1;
            }
            cout<<"received UDP : \n";
            try {
                n.process_datagram(p);
            } catch (myExceptions e) {
                e.printmsg();
                return 1;
            }
            
        }
        
        if (FD_ISSET(listener_stream, &set)) {
            cout<<"TCP :\n";
            addr_len = sizeof(remoteaddr);
            if ((newfd = accept(listener_stream,(struct sockaddr *)&remoteaddr,	&addr_len)) == -1) {
                perror("accept");
                return 1;
            }
            try {
                n.process_stream(newfd);
            } catch (myExceptions e) {
                e.printmsg();
                return 1;
            }
            
        }
        set = masterset;
        n.show();
    } while (true);
    
    
    try {
        n.leave();
    } catch (myExceptions e) {
        e.printmsg();
        return 1;
    }
    
    return 0;
}
