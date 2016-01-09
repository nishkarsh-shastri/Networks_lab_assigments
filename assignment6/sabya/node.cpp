//
//  node.cpp
//  Chord_0
//
//  Created by SABYASACHEE BARUAH on 28/03/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "declarations.hxx"

#define KEY 9


int file_select(const struct dirent *entry){
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
        return (0);
    else
        return (1);
}

// to be changed
int dohash(int port) {
    return port%10;
}

// to be changed
int dohash(string filename) {
    static int c = -1;
    c++;
    return c%10;
}

Node::Node() {}

Node::Node(string name) throw (myExceptions){
    int shmid, datagramport = -1, streamport, identifier, count;
    char folder[100];
    struct dirent **files;
    struct data *d;
    myExceptions e;
    
    if ((shmid = shmget(KEY, 200, 0666)) == -1) {
        
        shmid = shmget(KEY, 200, 0666 | IPC_CREAT);
        if ((d = (struct data*)shmat(shmid, 0, 0)) == NULL) {
            perror("attach");
            e.setmsg("Error in shared memory attachment");
            throw e;
        }
        
        d->portlist[0][0] = 10000; d->portlist[0][1] = 1;
        d->portlist[1][0] = 10002; d->portlist[1][1] = 0;
        d->portlist[2][0] = 10004; d->portlist[2][1] = 0;
        d->portlist[3][0] = 10006; d->portlist[3][1] = 0;
        d->portlist[4][0] = 10008; d->portlist[4][1] = 0;
        d->portlist[5][0] = 10010; d->portlist[5][1] = 0;
        d->portlist[6][0] = 10012; d->portlist[6][1] = 0;
        d->portlist[7][0] = 10014; d->portlist[7][1] = 0;
        d->portlist[8][0] = 10016; d->portlist[8][1] = 0;
        d->portlist[9][0] = 10018; d->portlist[9][1] = 0;
        d->port = datagramport = 10000;
        strcpy(d->name, (char*)name.c_str());
    }
    else {
        
        if ((d = (struct data*)shmat(shmid, 0, 0)) == NULL) {
            perror("attach");
            e.setmsg("Error in shared memory attachment");
            throw e;
        }
        for (int i = 0; i < 10; ++i) {
            if (d->portlist[i][1] == 0) {
                datagramport = d->portlist[i][0];
                d->portlist[i][1] = 1;
                break;
            }
        }
    }
    
    streamport = datagramport + 1;
    identifier = dohash(datagramport);
    enode = address(dohash(d->port), d->port, d->name);
    node = address(identifier, datagramport, name);
    datagram = Connect(DATAGRAM, datagramport);
    stream = Connect(STREAM, streamport);
    
    sprintf(folder, "/Users/Sabyasachee/%d", datagramport);
    strcat(folder, "shared");
    if (access(folder, F_OK) != 0) {
        if (mkdir(folder, 0777) == -1) {
            perror("mkdir");
            e.setmsg("Error in creating directory");
            throw e;
        }
    }
    
    count = scandir(folder, &files, file_select, alphasort);
    
    for (int i = 0; i < count; ++i) {
        filelist.push_back(files[i]->d_name);
    }
    state = NEW;
    shmdt(d);
    
}

void Node::join() throw (myExceptions) {
    
    int info, key;
    string file;
    Packet p;
    address m;
    myExceptions e;
    
    if (enode == node) {
        successor = node;
        predecessor = node;
        for (list<string>::iterator it = filelist.begin(); it != filelist.end(); ++it) {
            key = dohash(*it);
            set<address> s;
            s.insert(node);
            index.push_back(pair<int, set<address>>(key, s));
        }
        return;
    }
    
    p.setcmd(GETSUCC);
    p.setsource(node);
    p.setdestination(enode);
    info = get<0>(node);
    p.setdata(new int(info));
    datagram.send(p);
    p = datagram.recv();
    if (p.getcmd() != FOUNDSUCC) {
        e.setmsg("Incorrect Response 1");
        throw e;
    }
    successor = *(address*)(p.getdata());
    
    p.setcmd(GETPRED);
    p.setsource(node);
    p.setdestination(successor);
    datagram.send(p);
    p = datagram.recv();
    if (p.getcmd() != SENDPRED) {
        e.setmsg("Incorrect Response 2");
        throw e;
    }
    predecessor = *(address*)(p.getdata());
    
    p.setcmd(SETSUCC);
    p.setsource(node);
    p.setdestination(predecessor);
    p.setdata(&node);
    datagram.send(p);
    
    p.setcmd(SETPRED);
    p.setsource(node);
    p.setdestination(successor);
    p.setdata(&node);
    datagram.send(p);
    /*
    p.setcmd(GETINDEX);
    p.setdestination(successor);
    p.setsource(node);
    datagram.send(p);
    
    p = datagram.recv();
    if (p.getcmd() != SENDINDEX) {
        e.setmsg("Incorrect Response 3");
        throw e;
    }
    index = *(vector<pair<int, set<address>> >*)p.getdata();
    
    for (list<string>::iterator it = filelist.begin(); it != filelist.end(); it++) {
        file = *it;
        key = dohash(file);
        p.setcmd(GETSUCC);
        p.setsource(node);
        p.setdestination(successor);
        p.setdata(&key);
        datagram.send(p);
        p = datagram.recv();
        if (p.getcmd() != FOUNDSUCC) {
            e.setmsg("Incorrect Response 1");
            throw e;
        }
        m = *(address*)p.getdata();
        
        p.setcmd(SENDINFO);
        p.setsource(node);
        p.setdestination(m);
        p.setdata(&key);
        datagram.send(p);
    }*/
}


void Node::process_user(string input, address user, bool fileprompt) throw(myExceptions) {
    
    int key = dohash(input);
    int pred = get<0>(predecessor);
    int n = get<0>(node);
    int port = get<1>(node);
    char folder[20];
    int sockfd;
    Packet p;
    
    sprintf(folder, "%dshared/%s", port, (char*)input.c_str());
    if (access(folder, F_OK) != -1) {
        cout<<"file exists..";
        return;
    }
    
    if (fileprompt) {
        if (key > pred && key <= n) {
            for (vector<pair<int, set<address>> >::iterator it = index.begin(); it != index.end(); ++it) {
                if (key == (*it).first) {
                    p.setdata(&((*it).second));
                    break;
                }
            }
            p.setcmd(SENDUSERS);
            sendusers(p);
        }
        else {
            p.setcmd(GETSUCC);
            p.setsource(node);
            p.setdestination(successor);
            p.setdata(&key);
            datagram.send(p);
        }
    }else {
        p.setcmd(DOWNREQ);
        p.setsource(user);
        p.setdestination(node);
        p.setdata(&input);
        datagram.send(p);
        sockfd = stream.connect(user);
        stream.savefile(folder, sockfd);
    }
}

void Node::process_datagram(Packet p) throw(myExceptions) {
    
    cout<<"inside process_datagram\n";
    int cmd = p.getcmd();
    
    switch (cmd) {
        case GETSUCC:
            cout<<"GETSUCC\n";
            getsuccessor(p);
            break;
        
        case FOUNDSUCC:
            foundsuccessor(p);
            break;
            
        case GETPRED:
            getpredecessor(p);
            break;
            
        case SETPRED:
            setpredecessor(p);
            break;
            
        case SETSUCC:
            setsuccessor(p);
            break;
            
        case GETINDEX:
            getindex(p);
            break;
        
        case REQUSERS:
            requestusers(p);
            break;
            
        case SENDUSERS:
            sendusers(p);
            break;
            
        case DOWNREQ:
            downloadrequest(p);
            break;
            
        case SENDINFO:
            sendinfo(p);
            break;
            
        default:
            break;
    }
}

void Node::process_stream(int newfd) throw(myExceptions) {
    stream.sendfile(newfd);
}

void Node::show() {
    
    cout<<"successor : "<<get<0>(successor)<<":"<<get<1>(successor)<<":"<<get<2>(successor)<<endl;
    cout<<"predecessor : "<<get<0>(predecessor)<<":"<<get<1>(predecessor)<<":"<<get<2>(predecessor)<<endl;
    cout<<"node : "<<get<0>(node)<<":"<<get<1>(node)<<":"<<get<2>(node)<<endl;
    cout<<"enode : "<<get<0>(enode)<<":"<<get<1>(enode)<<":"<<get<2>(enode)<<endl;
    cout<<endl;
    for (vector<pair<int, set<address>> >::iterator it = index.begin(); it != index.end(); ++it) {
        cout<<(*it).first<<" : ";
        for (set<address>::iterator jt = (*it).second.begin(); jt != (*it).second.end(); ++jt) {
            cout<<get<1>(*jt)<<":"<<get<2>(*jt)<<" ";
        }
    }
    cout<<endl;
    for (list<string>::iterator it = filelist.begin(); it != filelist.end(); ++it) {
        cout<<*it<<endl;
    }
    cout<<endl;
}

void Node::getsuccessor(Packet p) throw (myExceptions) {
    
    cout<<"getsuccessor\n";
    address source = p.getsource();
    cout<<"got source";
    int key = *((int*)p.getdata());
    cout<<"got key\n";
    address destination = p.getdestinaiton();

    
    int pred = get<0>(predecessor);
    int n = get<0>(node);
    
    cout<<"key = "<<key<<" pred = "<<pred<<" n = "<<n<<endl;

    if ((key > pred && key <= n) || (pred == n)) {
        p.setcmd(FOUNDSUCC);
        p.setdata(&node);
        p.setsource(destination);
        p.setdestination(source);
        datagram.send(p);
    }
    else {
        p.setcmd(GETSUCC);
        p.setdata(&key);
        p.setdestination(successor);
        p.setsource(source);
        datagram.send(p);
    }
}

void Node::foundsuccessor(Packet p) throw (myExceptions) {
    
    address source = p.getsource();
    address destination = p.getdestinaiton();
    
    p.setcmd(REQUSERS);
    p.setdestination(source);
    p.setsource(node);
    datagram.send(p);
    
    
}

void Node::getpredecessor(Packet p) throw (myExceptions) {
    
    address source = p.getsource();
    address destination = p.getdestinaiton();
    p.setcmd(SENDPRED);
    p.setdestination(source);
    p.setsource(node);
    p.setdata(&predecessor);
    datagram.send(p);
    
}


void Node::setsuccessor(Packet p) {

    address successor = *(address*)p.getdata();
    this->successor = successor;
    
}

void Node::setpredecessor(Packet p) {
    
    address predecessor = *(address*)p.getdata();
    this->predecessor = predecessor;

}

void Node::getindex(Packet p)  throw (myExceptions){
    
    address source = p.getsource();
    int src = get<0>(source);
    vector<pair<int, set<address>> > myindex;
    
    for (vector<pair<int, set<address>> >::iterator it = index.begin(); it != index.end(); ++it) {
        int key = (*it).first;
        if (key <= src) {
            myindex.push_back(*it);
            index.erase(it);
        }
    }
    p.setcmd(SENDINDEX);
    p.setsource(node);
    p.setdestination(source);
    p.setdata(&myindex);
    datagram.send(p);
    
}

void Node::requestusers(Packet p) throw (myExceptions) {
    
    address source = p.getsource();
    int key = *(int*)p.getdata();
    for (vector<pair<int, set<address>> >::iterator it = index.begin(); it != index.end(); ++it) {
        if (key == (*it).first) {
            p.setdata(&((*it).second));
            break;
        }
    }
    p.setcmd(SENDUSERS);
    p.setsource(node);
    p.setdestination(source);
    datagram.send(p);
    
}

void Node::sendusers(Packet p) {
    
    set<address> userset = *(set<address>*)p.getdata();
    for (set<address>::iterator it = userset.begin(); it != userset.end(); ++it) {
        address user = *it;
        cout<<&user<<endl;
    }
    cout<<"Enter user : ";
}

void Node::downloadrequest(Packet p) {
    
    string filename = *(string*)p.getdata();
    stream.setfile(filename);
    
}

void Node::sendinfo(Packet p) {
    
    address source = p.getsource();
    int key = *(int*)p.getdata();
    
    for (vector<pair<int, set<address>> >::iterator it = index.begin(); it != index.end(); ++it) {
        if (key == (*it).first) {
            (*it).second.insert(source);
            break;
        }
    }
    
}

void Node::leave() throw (myExceptions) {
    datagram.shut();
    stream.shut();
}

int Node::getdatagramsocket() {
    return datagram.getsocket();
}

int Node::getstreamsocket() {
    return stream.getsocket();
}
















