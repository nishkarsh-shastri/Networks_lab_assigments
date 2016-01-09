//
//  Node.cpp
//  Chord_1
//
//  Created by SABYASACHEE BARUAH on 04/04/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "classes.h"

Node::Node(Address a, UDP& u, TCP& t):me(a), udp(u), tcp(t) {
    root =  ROOT + std::to_string(a.getport()) + "shared/";
    if (access(root.c_str(), F_OK) == -1) {
        if (mkdir(root.c_str(), 0777) == -1) {
            perror("mkdir");
            exit(1);
        }
    }
    else {
        struct dirent **files;
        int count = scandir(root.c_str(), &files, file_select, alphasort);
        for (int i = 0; i < count; ++i) {
            filelist.insert(files[i]->d_name);
        }
    }
    finger = std::vector<Address>(M);
}

void Node::show() {
    std::cout<<"me              : ";me.show();
    std::cout<<"predecessor     : ";predecessor.show();
    std::cout<<"finger table    : "<<std::endl;
    int i = 0;
    for (std::vector<Address>::iterator it = finger.begin(); it != finger.end(); ++it, ++i) {
        std::cout<<start(me.getid(), i)<<" : ";finger[i].show();
    }
    std::cout<<"index           : "<<std::endl;
    for (std::list<std::pair<int, std::list<Address>>>::iterator it = index.begin(); it != index.end(); ++it) {
        std::cout<<(*it).first<<" = ";
        std::list<Address> users = (*it).second;
        for (std::list<Address>::iterator jt = users.begin(); jt != users.end(); ++jt) {
            Address a = *jt;
            a.show();
        }
    }
    std::cout<<"filelist        : "<<std::endl;
    for (std::set<std::string>::iterator it = filelist.begin(); it != filelist.end(); ++it) {
        std::cout<<*it<<" = "<<hash(*it)<<std::endl;
    }
    std::cout<<std::endl;
}


void Node::download(std::string filename) {
    int key = hash(filename);
    std::string name;
    for (std::set<std::string>::iterator it = filelist.begin(); it != filelist.end(); ++it) {
        if (filename == *it) {
            std::cout<<"File already available with you :("<<std::endl;
            std::cout<<"Enter a new file to download : "<<std::endl;
            return;
        }
    }
    Address n = findsuccessor(key);
    std::list<Address> users;
    if (n == me) {
        for (std::list<std::pair<int, std::list<Address>>>::iterator it = index.begin(); it != index.end(); ++it) {
            if ((*it).first == key) {
                users = (*it).second;
                break;
            }
        }
    }
    else {
        Packet p(me, n, R, key);
        udp.send(p);
        std::string message = tcp.receive();
        char *cstring = (char*)malloc(1024), *tofree;
        strcpy(cstring, message.c_str());
        tofree = cstring;
        char* userlist[20];
        int nusers = 0;
        while ((userlist[nusers] = strsep(&cstring, ";")) != NULL) {
            if (strcmp(userlist[nusers], "")) {
                nusers++;
            }
        }
        for (int i = 0; i < nusers; ++i) {
            Address user(userlist[i]);
            users.push_back(user);
        }
        free(tofree);
    }
    std::cout<<"Following users have the file you have requested..."<<std::endl;
    for (std::list<Address>::iterator it = users.begin(); it != users.end(); ++it) {
        Address a(*it);
        std::cout<<a.getname()<<std::endl;
    }
    std::cout<<"Enter user name to download from : ";
    std::cin>>name;
    Address b;
    for (std::list<Address>::iterator it = users.begin(); it != users.end(); ++it) {
        Address a(*it);
        if (a.getname() == name) {
            b = *it;
            break;
        }
    }
    Packet p(me, b, D, filename);
    udp.send(p);
    tcp.receivefile(root + filename);
    filelist.insert(filename);
    std::cout<<"file downloaded successfully"<<std::endl;
    std::cout<<"Enter a new file to download : "<<std::endl;
    if (n == me) {
        for (std::list<std::pair<int, std::list<Address>>>::iterator it = index.begin(); it != index.end(); ++it) {
            if ((*it).first == key) {
                (*it).second.push_back(me);
                break;
            }
        }
    }
    else {
        Packet p(me, n, A, key);
        udp.send(p);
    }
}

void Node::process() {
    Packet p = udp.receive();
    switch (p.getcommand()) {
        case S: {
            Packet q(me, p.getsource(), SQ, finger[0]);
            udp.send(q);
            break;
        }
            
        case C: {
            int key = p.getkey();
            Address n = closest_preceding_finger(key);
            Packet q(me, p.getsource(), CQ, n);
            udp.send(q);
            break;
        }
            
        case F: {
            Address n = findsuccessor(p.getkey());
            Packet q(me, p.getsource(), FQ, n);
            udp.send(q);
            break;
        }
        
        case P: {
            Packet q(me, p.getsource(), PQ, predecessor);
            udp.send(q);
            break;
        }
         
        case SP: {
            predecessor = p.getaddress();
            break;
        }
            
        case U: {
            update_fingertable(p.getaddress(), p.getkey());
            break;
        }
            
        case I: {
            char buf[1000];
            strcpy(buf, "");
            std::list<std::pair<int, std::list<Address>>> newindex;
            for (std::list<std::pair<int, std::list<Address>>>::iterator it = index.begin(); it != index.end(); ++it) {
                if (!onrange((*it).first, predecessor.getid() + 1, me.getid())) {
                    std::list<Address> users = (*it).second;
                    char userbuf[1000], msgbuf[1000];
                    strcpy(userbuf, "");
                    for (std::list<Address>::iterator jt = users.begin(); jt != users.end(); ++jt) {
                        Address a = *jt;
                        strcat(userbuf, a.serialize().c_str());
                        strcat(userbuf, ";");
                    }
                    userbuf[strlen(userbuf)-1] = '\0';
                    sprintf(msgbuf, "%d:%s",(*it).first, userbuf);
                    strcat(buf, msgbuf);
                    strcat(buf, "|");
                }
                else {
                    newindex.push_back(*it);
                }
            }
            index = newindex;
            buf[strlen(buf)-1] = '\0';
            tcp.send(buf, predecessor);
            break;
        }
            
        case A: {
            int key = p.getkey();
            std::list<std::pair<int, std::list<Address>>>::iterator it;
            for (it = index.begin(); it != index.end(); ++it) {
                if ((*it).first == key) {
                    (*it).second.push_back(p.getsource());
                    break;
                }
            }
            if (it == index.end()) {
                std::list<Address> users;
                users.push_back(p.getsource());
                std::pair<int, std::list<Address>> entry(key, users);
                index.push_back(entry);
            }
            break;
        }
            
        case R: {
            int key = p.getkey();
            std::list<Address> users;
            for (std::list<std::pair<int, std::list<Address>>>::iterator it = index.begin(); it != index.end(); ++it) {
                if ((*it).first == key) {
                    users = (*it).second;
                    break;
                }
            }
            char userbuf[1000];
            strcpy(userbuf, "");
            for (std::list<Address>::iterator jt = users.begin(); jt != users.end(); ++jt) {
                Address a = *jt;
                strcat(userbuf, a.serialize().c_str());
                strcat(userbuf, ";");
            }
            userbuf[strlen(userbuf)-1] = '\0';
            tcp.send(userbuf, p.getsource());
            break;
        }
            
        case D: {
            tcp.sendfile(root + p.getfile(), p.getsource());
            break;
        }
            
        case V: {
            remove_node(p.getsource(), p.getkey(), p.getaddress());
            break;
        }
            
        case J: {
            std::string message = tcp.receive();
            char *cstring = (char*)malloc(1024), *tofree;
            strcpy(cstring, message.c_str());
            tofree = cstring;
            char* tokenlist[20];
            int ntokens = 0;
            while ((tokenlist[ntokens] = strsep(&cstring, "|")) != NULL) {
                if (strcmp(tokenlist[ntokens], "")) {
                    ntokens++;
                }
            }
            for (int i = 0; i < ntokens; ++i) {
                char* ustring = (char*)malloc(1024), *tofree, *keybuf;
                strcpy(ustring, tokenlist[i]);
                tofree = ustring;
                char* userlist[10];
                int nusers = 0;
                keybuf = strsep(&ustring, ":");
                while ((userlist[nusers] = strsep(&ustring, ";")) != NULL) {
                    if (strcmp(userlist[nusers], "")) {
                        nusers++;
                    }
                }
                int key = atoi(keybuf);
                std::list<Address> users;
                for (int j = 0; j < nusers; ++j) {
                    Address user(userlist[j]);
                    users.push_back(user);
                }
                std::pair<int, std::list<Address>> ele(key, users);
                index.push_back(ele);
                free(tofree);
            }
            free(tofree);
            break;
        }
            
        case B: {
            int key = p.getkey();
            Address m = p.getaddress();
            for (std::list<std::pair<int, std::list<Address>>>::iterator it = index.begin(); it != index.end(); ++it) {
                if ((*it).first == key) {
                    for (std::list<Address>::iterator jt = (*it).second.begin(); jt != (*it).second.end(); ++jt) {
                        if ((*jt) == m) {
                            (*it).second.erase(jt);
                            break;
                        }
                    }
                    if ((*it).second.size() == 0) {
                        index.erase(it);
                    }
                    break;
                }
            }
            break;
        }
            
        default:
            break;
    }
}

void Node::leave() {
    ::leave(me, finger[0]);
    for (std::set<std::string>::iterator kt = filelist.begin(); kt != filelist.end(); ++kt) {
        int key = hash(*kt);
        Address m = findsuccessor(key);
        if (m == me) {
            for (std::list<std::pair<int, std::list<Address>>>::iterator it = index.begin(); it != index.end(); ++it) {
                if ((*it).first == key) {
                    for (std::list<Address>::iterator jt = (*it).second.begin(); jt != (*it).second.end(); ++jt) {
                        if ((*jt) == me) {
                            (*it).second.erase(jt);
                            break;
                        }
                    }
                    if ((*it).second.size() == 0) {
                        index.erase(it);
                    }
                break;
                }
            }
        }
        else {
            Packet q(me, m, B, key, me);
            udp.send(q);
        }
    }
    if (me == finger[0]) {
        return;
    }
    Packet p(me, finger[0], SP, me);
    udp.send(p);
    for (int i = 0; i < M; ++i) {
        Address p = findpredecessor((me.getid() - (1<<i) + 1)%(1<<M));
        Packet q(me, p, V, i, finger[0]);
        udp.send(q);
    }
    p = Packet(me, finger[0], J);
    udp.send(p);
    char buf[1000];
    strcpy(buf, "");
    for (std::list<std::pair<int, std::list<Address>>>::iterator it = index.begin(); it != index.end(); ++it) {
        std::list<Address> users = (*it).second;
        char userbuf[1000], msgbuf[1000];
        strcpy(userbuf, "");
        for (std::list<Address>::iterator jt = users.begin(); jt != users.end(); ++jt) {
            Address a = *jt;
            strcat(userbuf, a.serialize().c_str());
            strcat(userbuf, ";");
        }
        userbuf[strlen(userbuf)-1] = '\0';
        sprintf(msgbuf, "%d:%s",(*it).first, userbuf);
        strcat(buf, msgbuf);
        strcat(buf, "|");
    }
    buf[strlen(buf)-1] = '\0';
    tcp.send(buf, finger[0]);
}

void Node::remove_node(Address n, int i, Address repl) {
    if (finger[i] == n) {
        finger[i] = repl;
        Packet q(n, predecessor, V, i, repl);
        udp.send(q);
    }
}

void Node::join(Address n) {
    if (n.getid() != -1) {
        init_fingertable(n);
        updateothers();
        Packet p(me, finger[0], I);
        udp.send(p);
        std::string message = tcp.receive();
        char *cstring = (char*)malloc(1024), *tofree;
        strcpy(cstring, message.c_str());
        tofree = cstring;
        char* tokenlist[20];
        int ntokens = 0;
        while ((tokenlist[ntokens] = strsep(&cstring, "|")) != NULL) {
            if (strcmp(tokenlist[ntokens], "")) {
                ntokens++;
            }
        }
        for (int i = 0; i < ntokens; ++i) {
            char* ustring = (char*)malloc(1024), *tofree, *keybuf;
            strcpy(ustring, tokenlist[i]);
            tofree = ustring;
            char* userlist[10];
            int nusers = 0;
            keybuf = strsep(&ustring, ":");
            while ((userlist[nusers] = strsep(&ustring, ";")) != NULL) {
                if (strcmp(userlist[nusers], "")) {
                    nusers++;
                }
            }
            int key = atoi(keybuf);
            std::list<Address> users;
            for (int j = 0; j < nusers; ++j) {
                Address user(userlist[j]);
                users.push_back(user);
            }
            std::pair<int, std::list<Address>> ele(key, users);
            index.push_back(ele);
            free(tofree);
        }
        free(tofree);
        for (std::set<std::string>::iterator jt = filelist.begin(); jt != filelist.end(); ++jt) {
            int key = hash(*jt);
            Address m = findsuccessor(key);
            if (m == me) {
                std::list<std::pair<int, std::list<Address>>>::iterator it;
                for (it = index.begin(); it != index.end(); ++it) {
                    if ((*it).first == key) {
                        (*it).second.push_back(me);
                        break;
                    }
                }
                if (it == index.end()) {
                    std::list<Address> users;
                    users.push_back(me);
                    std::pair<int, std::list<Address>> entry(key, users);
                    index.push_back(entry);
                }
            }
            else {
                Packet p(me, m, A, key);
                udp.send(p);
            }
        }
    }
    else {
        for (std::vector<Address>::iterator it = finger.begin(); it != finger.end(); ++it) {
            *it = me;
        }
        predecessor = me;
        for (std::set<std::string>::iterator jt = filelist.begin(); jt != filelist.end(); ++jt) {
            int key = hash(*jt);
            std::list<std::pair<int, std::list<Address>>>::iterator it;
            for (it = index.begin(); it != index.end(); ++it) {
                if ((*it).first == key) {
                    (*it).second.push_back(me);
                    break;
                }
            }
            if (it == index.end()) {
                std::list<Address> users;
                users.push_back(me);
                std::pair<int, std::list<Address>> entry(key, users);
                index.push_back(entry);
            }
        }
    }
}


Address Node::findsuccessor(int key) {
    Address n = findpredecessor(key);
    if (n == me) {
        return finger[0];
    }
    else {
        Packet p(me, n, S);
        udp.send(p);
        do {
            p = udp.receive();
        } while(p.getcommand() != SQ);
        return p.getaddress();
    }
}

Address Node::findpredecessor(int key) {
    Address n = me;
    Address x = finger[0];
    while (onrange(key, n.getid() + 1, x.getid()) == false) {
        if (n == me) {
            n = closest_preceding_finger(key);
        }
        else {
            Packet p(me, n, C, key);
            udp.send(p);
            do {
                p = udp.receive();
            } while(p.getcommand() != CQ);
            n = p.getaddress();
        }
        if (n == me) {
            x = finger[0];
        }
        else {
            Packet p(me, n, S);
            udp.send(p);
            do {
                p = udp.receive();
            } while(p.getcommand() != SQ);
            x = p.getaddress();
        }
    }
    return n;
}

Address Node::closest_preceding_finger(int key) {
    for (int i = M - 1; i >= 0; --i) {
        int x = finger[i].getid();
        if (onrange(x, me.getid() + 1, key - 1)) {
            return finger[i];
        }
    }
    return me;
}

void Node::init_fingertable(Address n) {
    Packet p(me, n, F, me.getid() + 1);
    udp.send(p);
    do {
        p = udp.receive();
    } while(p.getcommand() != FQ);
    finger[0] = p.getaddress();
    p = Packet(me, finger[0], P);
    udp.send(p);
    do {
        p = udp.receive();
    } while(p.getcommand() != PQ);
    predecessor = p.getaddress();
    p = Packet(me, finger[0], SP, me);
    udp.send(p);
    for (int i = 0; i < M - 1; ++i) {
        if (onrange(start(me.getid(), i+1), me.getid(), finger[i].getid() - 1)) {
            finger[i+1] = finger[i];
        }
        else {
            Packet p(me, n, F, start(me.getid(), i+1));
            udp.send(p);
            do {
                p = udp.receive();
            } while(p.getcommand() != FQ);
            finger[i + 1] = p.getaddress();
            if (onrange(finger[i + 1].getid(), start(me.getid(), i+1), me.getid()) == false) {
                finger[i + 1] = me;
            }
        }
    }
}

void Node::updateothers() {
    for (int i = 0; i < M; ++i) {
        Address a = findpredecessor((me.getid() - (1<<i) + 1)%(1<<M));
        if (a == me) {
            return;
        }
        Packet p(me, a, U, i, me);
        udp.send(p);
    }
}

void Node::update_fingertable(Address s, int i) {
    if (s == me) {
        return;
    }
    if (onrange(s.getid(), me.getid() + 1, finger[i].getid())) {
        if (onrange(s.getid(), start(me.getid(), i), finger[i].getid()-1)) {
            finger[i] = s;
        }
        Packet p(me, predecessor, U, i, s);
        udp.send(p);
    }
}



