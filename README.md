# Instant-messaging
 
To run the project :

type "make" before to compile the source (in Project)

1:	
	Run the server by clicking on the "Project" folder then open a terminal
	Make "./server 2000" with 2000 the name of the port (2001,2002,... or other)

2:
	Run the client program corresponding to a person connecting
	by clicking on the "Project" folder then open a terminal:
	"./client localhost 2000" with localhost (or ip address of the server - host machine) and 2000 for ex or the name of the chosen port)


NB: The maximum number of clients that can connect to the server is 20 by default, set with NB_WORKERS in the source code of server.c at the beginning of the file.


When you connect on the client side, it asks for your name: 
	put "marc" for example.

Tip: 
	Start 2 clients with two different names to be able to communicate and test the program.

Once the two people are connected (before choosing the recipient), it is possible to communicate:
	Choose the name of the recipient in both clients
	Type the command lines to talk
	
	A communication end "client" -> recipient is done with "fin".

To stop a client :
	type "stop" when asked "Destinataire? : "


