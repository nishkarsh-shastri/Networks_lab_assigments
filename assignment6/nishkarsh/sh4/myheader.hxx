#include <stdio.h>
#include <iostream>
#include <cerrno>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

string to_string(int x)
{
	char str[15];
	sprintf(str, "%d", x);
	string s(str);
	return s;
}
string to_string(unsigned long long int x)
{
	char str[30];
	sprintf(str, "%lld", x);
	string s(str);
	return s;
}
string to_string(double x)
{
	char str[15];
	sprintf(str, "%lf", x);
	string s(str);
	return s;
}
string to_string(char x)
{
	char str[5];
	sprintf(str, "%c", x);
	string s(str);
	return s;
}
string to_string(string x)
{
	return x;
}

unsigned long long oat_hash(const char *p, int len)
{
    
    unsigned long long h = 0;
    int i;

    for (i = 0; i < len; i++)
    {
        h += p[i];
        h += (h << 10);
        h ^= (h >> 6);
    }

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);

    return h;
}

class Node
{
	public:
		Node(string _myip, int _myport=0);
		Node(Node&);
		// get set fucntions for successor and predecessor
		Node* getSuccessor();
		Node* getPredecessor();
		void setSuccessor(Node*);
		void setPredecessor(Node*);
		string getIp();	
		int getPort();
		unsigned long long getMachineId();
		void getStatus();
	private:
		int myport;
		string myip;
		unsigned long long my_machine_id;
		Node* successor;
		Node* predecessor;
};


void Node::getStatus()
{
	cout<<"My MachineId :: "<<my_machine_id<<endl;
    cout<<"successor MachineId :: "<<successor->getMachineId()<<endl;
    cout<<"predecessor MachineId :: "<<predecessor->getMachineId()<<endl;
}
Node::Node(string _myip, int _myport)
{
	myip = _myip;
	myport = _myport;
	string temp;
	temp = myip+":";
	temp = temp + to_string(myport);
	cout<<"IP AND PORT ARE "<<temp<<endl;
	int l = temp.length();
	my_machine_id = oat_hash(temp.c_str(), l)%32;
	cout<<"My machine id is: "<<my_machine_id<<endl;
	successor = NULL;
	predecessor = NULL;
}

Node::Node(Node& node)
{
	myip = node.myip;
	myport = node.myport;
	my_machine_id = node.my_machine_id;
	successor =  node.successor;
	predecessor = node.predecessor;
}
Node* Node::getSuccessor()
{
	return successor;
}
Node* Node::getPredecessor()
{
	return predecessor;
}
void Node::setSuccessor(Node* s)
{
	successor = s;
}
void Node::setPredecessor(Node* p)
{
	predecessor = p;
}
string Node::getIp()
{
	return myip;
}
int Node::getPort()
{
	return myport;
}
unsigned long long Node::getMachineId()
{
	return my_machine_id;
}



class Command
{
	public:
		string com;
		int source_machine_port;
		int source_machine_id;		
		int destination_machine_port;
		int destination_machine_id;
		string data;
		int hash;	
};