//
//  functions.cpp
//  Chord_1
//
//  Created by SABYASACHEE BARUAH on 04/04/15.
//  Copyright (c) 2015 SABYASACHEE BARUAH. All rights reserved.
//

#include "headers.h"
#include "classes.h"

int hash(std::string message)
{
    message = "127.0.0.1:" + message;
    char* p = (char*)message.c_str();
    long len = message.length();
    unsigned long long h = 0;
    for (int i = 0; i < len; i++) {
        h += p[i];
        h += (h << 10);
        h ^= (h >> 6);
    }
    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);
    return h%(1<<M);
}

int file_select(const struct direct *entry) {
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) || (strcmp(entry->d_name, ".DS_Store") == 0))
        return 0;
    else return 1;
}

int create() {
    int shmid;
    if ((shmid = shmget(KEY, 1024, 0666)) == -1) {
        if ((shmid = shmget(KEY, 1024, 0666 | IPC_CREAT)) == -1) {
            perror("shmget");
            exit(1);
        }
        struct shared *data;
        if ((data = (struct shared*)shmat(shmid, 0, 0)) == NULL) {
            perror("shmat");
            exit(1);
        }
        data->ports[0] = 9004;data->available[0] = 1;
        data->ports[1] = 9003;data->available[1] = 1;
        data->ports[2] = 9009;data->available[2] = 1;
        for (int i = 3; i < 1<<M; ++i) {
            data->ports[i] = 12000 + 2*i;
            data->available[i] = 1;
        }
        data->nport = -1;
        shmdt(data);
    }
    return shmid;
}

std::tuple<int, Address> getport_address(int shmid, std::string name) {
    struct shared *data;
    int port = -1;
    Address addr;
    if ((data = (struct shared*)shmat(shmid, 0, 0)) == NULL) {
        perror("shmat");
        exit(1);
    }
    for (int i = 0; i < 1<<M; ++i) {
        if (data->available[i] == 1) {
            data->available[i] = 0;
            port = data->ports[i];
            break;
        }
    }
    if (data->nport != -1) {
        addr = Address(data->nport, data->nname);
    }
    else {
        strcpy(data->nname, name.c_str());
        data->nport = port;
    }
    shmdt(data);
    return std::tuple<int, Address>(port, addr);
}

void leave(Address leaving, Address existing) {
    struct shared *data;
    int shmid;
    shmid = shmget(KEY, 1024, 0666);
    if ((data = (struct shared*)shmat(shmid, 0, 0)) == NULL) {
        perror("shmat");
        exit(1);
    }
    if (data->nport == leaving.getport()) {
        if (leaving == existing) {
            data->nport = -1;
        }
        else {
            data->nport = existing.getport();
            strcpy(data->nname, existing.getname().c_str());
        }
    }
    for (int i = 0; i < (1<<M); ++i) {
        if (data->ports[i] == leaving.getport()) {
            data->available[i] = 1;
            break;
        }
    }
    shmdt(data);
}

bool onrange(int x, int a, int b) {
    if (a <= b) {
        return a <= x && x <= b;
    }
    else {
        return x >= a || x <= b;
    }
}

std::string comm(int command) {
    switch (command) {
        case 1:
            return "get successor";
            break;
            
        case 2:
            return "closest preceding finger";
            break;
            
        case 3:
            return "find successor";
            break;
            
        case 4:
            return "get predecessor";
            break;
            
        case 5:
            return "set predecessor";
            break;
        
        case 6:
            return "update finger table";
            break;
            
        case 7:
            return "transfer index";
            break;
            
        case 8:
            return "add user";
            break;
            
        case 9:
            return "give users set";
            break;
            
        case 97:
            return "remove node";
            break;
            
        case 98:
            return "transferring keys to successor";
            break;
            
        case 99:
            return "download file request";
            break;
            
        case 10:
            return "normal reply";
            break;
            
        case 11:
            return "reply to get successor";
            break;
            
        case 12:
            return "reply to closest preceding finger";
            break;
            
        case 13:
            return "reply to find successor";
            break;
            
        case 14:
            return "reply to get predecessor";
            break;
            
        default:
            return "something wrong";
            break;
    }
}