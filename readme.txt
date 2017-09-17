There are 2 codes in this programming assignment:
1) manneudps.c (server)
2) manneudpc.c (client)


1)sever side: 

-->to compile the server code use: 
              gcc -std=c99 manneudps.c -o manneudps

-->to run the server code use:
              ./manneudps <port no> <Gateway> <subnet mask>
             eg:  ./manneudps 50014 192.168.20.2 255.255.255.0


2)client side:

 --> To compile the server code use :
              gcc -std=c99 manneudpc.c -o manneudpc

--> TO run the client code
              ./manneudpc 50014 

We first run the server and then the client.
after client is run 0.0.0.0 is sent to server with a random transaction ID
At the server side we give gateway and subnet mask, accordingly server sends 3 ip addresses to the client.
client then selects a random IP address from the 3 ip addresses given by the server.
Server then allots the ip address and send a confirmation to the client with a new transaction id.

              
