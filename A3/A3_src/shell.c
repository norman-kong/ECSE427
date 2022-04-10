
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "interpreter.h"
#include "shellmemory.h"
#include "kernel.h"

#define FRAME_SIZE 3 // each page/frame is 3 lines of code
#define NUM_FRAMES 6
#define FRAME_STORE_SIZE (FRAME_SIZE*NUM_FRAMES)
#define VAR_STORE_SIZE 10 // last VAR_STORE_SIZE lines of the memory are used for tracking variables

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {

	printf("%s\n", "Shell version 1.1 Created January 2022");
	help();
	printf("Frame Store Size = %d; Variable Store Size = %d", FRAME_STORE_SIZE, VAR_STORE_SIZE);

	char prompt = '$';  				// Shell prompt
	char userInput[MAX_USER_INPUT];		// user's input stored here
	int errorCode = 0;					// zero means no error, default

	//init user input
	for (int i=0; i<MAX_USER_INPUT; i++)
		userInput[i] = '\0';
	
	mem_init(); //init shell memory
	PCBs_init(); //init PCBs
	rq_init(); 

	while(1) {							
		printf("%c ",prompt);
		fgets(userInput, MAX_USER_INPUT-1, stdin);

		// 1.2.3
		if (feof(stdin) != 0) {
			freopen("/dev/tty", "r", stdin); // redirects stdin to the terminal for the current process
		} 

		// 1.2.5
		char* commands = strtok(userInput, ";"); 
		while (commands) {
			errorCode = parseInput(commands);
			if (errorCode == -1) exit(99);	// ignore all other errors
			memset(commands, 0, sizeof(commands));
			commands = strtok(NULL, ";"); // get next command
		}
	}

	return 0;

}

// Extract words from the input then call interpreter
int parseInput(char ui[]) {
 
	char tmp[200];
	char *words[100];							
	int a,b;							
	int w=0; // wordID

	for(a=0; ui[a]==' ' && a<1000; a++);		// skip white spaces

	while(ui[a] != '\0' && a<1000) {

		for(b=0; ui[a]!='\0' && ui[a]!=' ' && a<1000; a++, b++)
			tmp[b] = ui[a];						// extract a word
	 
		tmp[b] = '\0';

		words[w] = strdup(tmp);

		w++;
		
		if(ui[a] == '\0'){
			break;
		}
		a++; 
	}

	return interpreter(words, w);
}
