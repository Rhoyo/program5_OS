#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/wait.h>

#define SERVER_ID 1

void error(const char *msg) {
	perror(msg);
	exit(1);
} 

void setupAddressStruct(struct sockaddr_in* address, int portNumber){
	 
	// Clear out the address struct
	memset((char*) address, '\0', sizeof(*address)); 
	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);
	// Allow a client at any address to connect to this server
	address->sin_addr.s_addr = INADDR_ANY;
}

char* encrypt(char* plain, char* key){
	char* cipher;
	cipher=malloc(strlen(plain)+1);
	

	for(int i=0; i< strlen(plain); i++){
		//convert ' '  to [ for calculations
		if(plain[i]==32)
			plain[i]=91;

		if(key[i]==32)
			key[i]=91;
		
		char p= plain[i]-65;
		char k= key[i]-65;

		//this is the ciphered alphabet #
		char c= (p+k)%27;

		cipher[i]=c+65;
		
		//convert [ to ' ' into cipher
		if(cipher[i]==91)
			cipher[i]=32;
	}
	cipher[strlen(plain)]='\0';

	return cipher;
}

void genCipher(int connectionSocket){

	int charsWritten, charsRead;

	int sID=SERVER_ID;
	char* key;
	char* plain;
	int kLen, pLen;

	char* cipher;

	//verify 
	charsWritten= send(connectionSocket, &sID, sizeof(sID), 0);
	if(charsWritten <0)
		error("Error sending sID");


	//receive plaintxt length
	pLen=-1;
	charsRead= recv(connectionSocket, &pLen, sizeof(pLen),0);	
	if(charsRead <0)
		error("Error receiving plaintxt length");


	//receive key length
	kLen=-1;
	charsRead= recv(connectionSocket, &kLen, sizeof(kLen),0);	
	if(charsRead <0)
		error("Error receiving key length");

	
	//setup plaintext for receiving
	plain=NULL;
	key=NULL;
	plain=malloc(pLen);
	key=malloc(kLen);
	
	//receive plain text	
	charsRead= recv(connectionSocket, plain, pLen,0);	
	if(charsRead <0)
		error("Error receiving plain");


	//receive key
	charsRead= recv(connectionSocket, key, kLen,0);	
	if(charsRead <0)
		error("Error receiving key");


	//encrypt plaintext and set up cipher
	cipher=malloc(pLen);
	cipher=encrypt(plain, key);


	//send the cipher to client
	charsWritten= send(connectionSocket, cipher, strlen(cipher), 0);
	if(charsWritten <0)
		error("Error sending cipher");


	free(plain);
	free(key);
	free(cipher);
	return;
}



int main(int argc, char *argv[]){
	int connectionSocket;

	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);

	// Check usage & args
	if (argc < 2) { 
	fprintf(stderr,"USAGE: %s port\n", argv[0]); 
	exit(1);
	}
	
	 
	// Create the socket that will listen for connections
	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket < 0)
	error("ERROR opening socket");
			  
	// Set up the address struct for the server socket
	setupAddressStruct(&serverAddress, atoi(argv[1]));

	if(bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	    error("ERROR on binding");
		  
	// Start listening for connetions. Allow up to 5 connections to queue up
	listen(listenSocket, 5); 
			//     
	// Accept a connection, blocking if one is not available until one connects
	while(1){
	// Accept the connection request which creates a connection socket
	connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
	if (connectionSocket < 0)
		error("ERROR on accept");
  	printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr),ntohs(clientAddress.sin_port));
	

	//create child processes
	switch(fork()){
		case -1:
			error("Couldn't create child process\n");
		case 0:
				genCipher(connectionSocket);
				break;
		default:
			wait(NULL);		
	}

	 // Close the connection socket for this client
	close(connectionSocket); 
	}
	// Close the listening socket
	close(listenSocket);
	return 0;
}
