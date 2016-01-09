How to run:
Open the terminal and run make
open separate terminals
In one of the terminal run ./a.out which would be running the central server
In other ones run ./client localhost <sharedFolderName> <downloadFolderName>
You can run atmost 32 terminals. This is hardcoded and can be easily changed from changing the oat_hash%32 taken in centralServer.cpp and node_client.cpp


How it works?
1) Node request predecessor and successor from central server. This is the only use of central server.
2) Node sends information to its predecessor and successor that it has joined.
3) After all the nodes have joined. Write "file" in each of the client terminals. This will upload the keys of all the files in the shared folder. 
4) Now you can see the keys in other terminals. You just have to write the fileName to download that from some other server.


