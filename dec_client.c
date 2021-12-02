#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define CLIENT_ID 0

void error(const char *msg) { 
	  perror(msg); 
	  exit(0); 
} 

void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname){
	 // Clear out the address struct
	memset((char*) address, '\0', sizeof(*address));    
		 
	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);

	//get DNS
	struct hostent* hostInfo = gethostbyname(hostname); 
  	if(hostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0); 
	}
	//copy the first IP address from DNS entry to sin_addr.s_addr
	memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

char* readF(char* fName){
	char* text=NULL;
	int len=0;
	size_t size=-1;
	
	//grab file from fName
	FILE* file= fopen(fName, "r");
	if(file==NULL){
		fclose(file);
		error("Bad file\n");
	}
	
	//get txt from file
	len=getline(&text, &size, file);

	//if a char in the file is not a space or in range of 65 to 90, file is bad
	for(int i=0; i< len-1; i++){
		if(text[i]!=32 && (text[i] < 65 || text[i] > 90))
			error("Error: Bad txt file\n");
	}
	if(text[len-1]=='\n')
		text[len-1]='\0';
	

	fclose(file);
	return text;
}

int main(int argc, char *argv[]) {
	int socketFD, portNumber, charsWritten, charsRead, sID;
	struct sockaddr_in serverAddress;
	char* plain;
	int cLen;
	char* key;
	int kLen;
	char* cipher;


	if(argc < 4) { 
		fprintf(stderr, "USAGE: %s ciphertext key port\n", argv[0]); 
		exit(0); 
	 }

	cipher=readF(argv[1]);
	key=readF(argv[2]);

	if(strlen(key) < strlen(cipher))
		error("CLIENT: ERROR key length too short\n"); 

	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0)
		error("CLIENT: ERROR opening socket");



	
	//setup address struct
	setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

	 // Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		fprintf(stderr, "CLIENT: ERROR connecting to port %i", atoi(argv[3]));
		exit(2);
	}


	//read sID
	sID=-1;
	charsRead = recv(socketFD, &sID, sizeof(sID), 0);
	if (charsRead < 0)
		error("CLIENT: ERROR reading from socket");



	//verifications
	if(sID!=CLIENT_ID)
		error("Client: ERROR incompatible client and server\n");
	
	//set up kLen and cLen for sending
	kLen=strlen(key);
	cLen=strlen(cipher);

	//send pLen
	charsWritten = send(socketFD, &cLen, sizeof(cLen), 0); 
	if (charsWritten < 0)
		error("CLIENT: ERROR writing pLen to socket");


	//send kLen
	charsWritten = send(socketFD, &kLen, sizeof(kLen), 0); 
	if (charsWritten < 0)
		error("CLIENT: ERROR writing kLen to socket");


	//send cipher
	charsWritten = send(socketFD, cipher, strlen(cipher), 0); 
	if (charsWritten < 0)
		error("CLIENT: ERROR writing plain to socket");
	
	if (charsWritten < strlen(cipher))
		printf("CLIENT: WARNING: Not all data written to socket!\n");

	//send key
	charsWritten = send(socketFD, key, strlen(key), 0); 
	if (charsWritten < 0)
		error("CLIENT: ERROR writing key to socket");
	
	if (charsWritten < strlen(key))
		printf("CLIENT: WARNING: Not all data written to socket!\n");
	
	
	
	//read data from socket, leave \0 at end
	plain=malloc(cLen+1);
	charsRead = recv(socketFD, plain, strlen(cipher)+1, 0); 
	if (charsRead < 0)
		error("CLIENT: ERROR reading cipher from socket");
	plain[cLen]='\0';
	
	printf("%s\n", plain);

	//close socket
	close(socketFD);
	free(plain);
	return 0;
}
