
Author  - Sabyasachee

Some implementation details:

1.
	Every node is associated with a shared folder that contains the files it shares as well as the 
	files it downloads. It is given the name PORT#shared where PORT# is the port number of the 
	node that is guaranteed to be different.

Some simplifying assumptions of the implementation:

1.
	The value associated with the key is in general a set of IP addresses that has the file. 
	We assume that a file is present at only one node. And when other nodes download the file,
	they don't get shared or the value set is not modified for that key. The value set for our
	case is singleton.

2.
