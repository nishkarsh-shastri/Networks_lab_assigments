//
//  main.cpp
//  Chord_1
//
//  Created by SABYASACHEE BARUAH on 03/04/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "classes.h"

int main(int argc, const char * argv[]) {
    int shmid = create();
    std::string name, message;
    std::cout<<"Enter name to join P2P Chord : ";
    std::cin>>name;
    std::pair<int, Address> m = getport_address(shmid, name);
    int udpport = m.first;
    int tcpport = udpport + 1;
    UDP udp(udpport);
    TCP tcp(tcpport);
    Address me(udpport, name);
    Node n(me, udp, tcp);
    n.join(m.second);
    std::cout<<"Successfully joined P2P Chord..."<<std::endl;
    fd_set set, read;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    FD_SET(udp.getsocket(), &set);
    int fdmax = udp.getsocket();
    n.show();
    std::cout<<"To download a file Enter filename and press Enter"<<std::endl;
    std::cout<<"To see your status and fingertable Press S (or s) and press Enter"<<std::endl;
    std::cout<<"To quit Press Q (or q) and press Enter\n"<<std::endl;
    fflush(stdout);
    fflush(stdin);
    while (true) {
        read = set;
        if (select(fdmax+1, &read, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }
        if (FD_ISSET(STDIN_FILENO, &read)) {
            std::cin>>message;
            if (message == "Q" || message == "q") {
                break;
            }
            else if (message == "S" || message == "s") {
                n.show();
            }
            else n.download(message);
        }
        if (FD_ISSET(udp.getsocket(), &read)) {
            n.process();
        }
        fflush(stdout);
        fflush(stdin);
    }
    n.leave();
    return 0;
}
