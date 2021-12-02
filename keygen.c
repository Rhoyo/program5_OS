#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>  // ssize_t


int main(int argc, char* argv[]){
	if(!argv[1] || argv[1]<=0){
		printf("Invalid key length!\n");
		exit(0);
	}
	
	ssize_t len= atoi(argv[1]);
	char key[len+1];

	memset(key, '\0', sizeof(key));
	key[len+1]='\n';
	
	srand(time(NULL));
	for(int i=0; i<len; i++){
		int x=65+rand()%27;
		if(x==91)
			x=32;
		key[i]=x;		//this is 
		printf("%c", key[i]);
	}
	printf("%c", key[len+1]);
}
