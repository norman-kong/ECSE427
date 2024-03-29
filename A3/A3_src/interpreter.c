#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h> // for isalnum
#include <sys/stat.h> // for mkdir 
#include <errno.h> // for mkdir

#include "shellmemory.h"
#include "shell.h"
#include "kernel.h"

int help();
int quit();
int badcommand();
int check_alnum(char* str);
int echo(char* command_arg);
int set(char* var, char* value);
int print(char* var);
int my_ls();
int run(int args_size, char* scripts[]);
int badcommandFileDoesNotExist();
int scheduler(int arg_size, char* scripts[], char policy[]);

#define MAX_USER_INPUT 1000

void clear_backing_store() {
	char remove[100] = "rm -rf backing_store"; 
	system(remove);	
}

// tries to clear the backing store. 
void init_backing_store() {
	int err = mkdir("backing_store", 0777);
	if (err == -1) {
		if (strcmp(strerror(errno), "File exists") == 0) {
			puts("Directory backing_store exists. Clearing old content.");
			clear_backing_store();
			int err = mkdir("backing_store", 0777);
			if (err == -1) {
				puts("Problem creating backing_store");
			}
		}
	}
}

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size){
	int i;

	for ( i=0; i<args_size; i++){ //strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommand();
	    return help();
	
	} else if (strcmp(command_args[0], "quit")==0) {
		if (args_size != 1) return badcommand();
		clear_backing_store();
		return quit();

	} else if (strcmp(command_args[0], "set")==0) {
		//set

		// if time: check all command args to be alphanumeric at the beginning, for cleaner code

		char *key = command_args[1];
		char *value = malloc(500*sizeof(char)); // 500 because tokens will be <100 chars each

		if (args_size < 3) {
			return badcommand();	
		} else if (args_size == 3) {
			if (check_alnum(command_args[2])) {
				strcpy(value, command_args[2]);
			} else {
				return badcommand();
			}
		} else if (args_size == 4) { // 2 tokens
			for (int i=2; i<=3; i++) {
				if (check_alnum(command_args[i])) {
					strcat(value, command_args[i]);
					strcat(value, " ");
				} else {
					return badcommand();
				}
			}
		} else if (args_size == 5) {
			for (int i=2; i<=4; i++) {
				if (check_alnum(command_args[i])) {
					strcat(value, command_args[i]);
					strcat(value, " ");
				} else {
					return badcommand();
				}
			}
		} else if (args_size == 6) {
			for (int i=2; i<=5; i++) {
				if (check_alnum(command_args[i])) {
					strcat(value, command_args[i]);
					strcat(value, " ");
				} else {
					return badcommand();
				}
			}
		} else if (args_size == 7) {
			for (int i=2; i<=6; i++) {
				if (check_alnum(command_args[i])) {
					strcat(value, command_args[i]);
					strcat(value, " ");
				} else {
					return badcommand();
				}
			}
		} else {
			printf("%s\n", "Bad command: Too many tokens");
			return 1;
		}
		return set(key, value); 
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand();
		init_backing_store();

		// save to backing store
		char copy[100] = "cp "; 
		strcat(copy, command_args[1]); // source
		strcat(copy, " backing_store/");
		strcat(copy, command_args[1]); // destination
		system(copy);	

		/*
		for (int i=0; i<args_size; i++) {
			puts(command_args[i]);
		}*/
 
		return run(args_size, command_args);
	
	} else if (strcmp(command_args[0], "echo")==0) {
		if (args_size != 2) return badcommand();
		return echo(command_args[1]);
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size != 1) return badcommand();
		return my_ls();
	} else if (strcmp(command_args[0], "exec")==0) {

		init_backing_store();

		// save to backing store
		for (int i=1; i<args_size-1; i++) {
			char copy[100] = "cp "; 
			strcat(copy, command_args[i]); // source
			strcat(copy, " backing_store/");
			strcat(copy, command_args[i]); // destination
			puts(copy);
			system(copy);	
		}

		if (args_size < 3) {
			return badcommand();
		} else if (args_size == 3) { // exec 1 program (same as run)
			if (strcmp(command_args[2], "FCFS") == 0) {
				scheduler(args_size, command_args, "FCFS");
			} else if (strcmp(command_args[2], "SJF") == 0) {
				scheduler(args_size, command_args, "FCFS");
			} else if (strcmp(command_args[2], "RR") == 0) {
				scheduler(args_size, command_args, "FCFS");
			} else if (strcmp(command_args[2], "AGING") == 0) {
				scheduler(args_size, command_args, "FCFS");
			} else {
				return badcommand();
			}
		} else if (args_size == 4) { // exec 2 programs

			/* REMOVED FOR A3 - DUPLICATES NOW ALLOWED
			if (strcmp(command_args[1], command_args[2]) == 0){
				printf("%s\n", "Bad command: same file name");
				return 1;
			}
			*/

			if (strcmp(command_args[3], "FCFS") == 0) {
				scheduler(args_size, command_args, "FCFS");
			} else if (strcmp(command_args[3], "SJF") == 0) {
				scheduler(args_size, command_args, "SJF");
			} else if (strcmp(command_args[3], "RR") == 0) {
				scheduler(args_size, command_args, "RR");
			} else if (strcmp(command_args[3], "AGING") == 0) {
				scheduler(args_size, command_args, "AGING");
			} else {
				return badcommand();
			}
		} else if (args_size == 5) { // exec 3 programs

			/* REMOVED FOR A3 - DUPLICATES NOW ALLOWED
			if (strcmp(command_args[1], command_args[2]) == 0){
				printf("%s\n", "Bad command: same file name");
				return 1;
			} else if (strcmp(command_args[2], command_args[3]) == 0){
				printf("%s\n", "Bad command: same file name");
				return 1;
			} else if (strcmp(command_args[1], command_args[3]) == 0){
				printf("%s\n", "Bad command: same file name");
				return 1;
			}
			*/
		
			if (strcmp(command_args[4], "FCFS") == 0) {
				scheduler(args_size, command_args, "FCFS");
			} else if (strcmp(command_args[4], "SJF") == 0) {
				scheduler(args_size, command_args, "SJF");
			} else if (strcmp(command_args[4], "RR") == 0) {
				scheduler(args_size, command_args, "RR");
			} else if (strcmp(command_args[4], "AGING") == 0) {
				scheduler(args_size, command_args, "AGING");
			} else {
				return badcommand();
			}
		} else {
			return badcommand();
		}

	} else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit(){
	printf("%s\n", "Bye!");
	exit(0);
}

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

// For run command only
int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

// helper function that checks if a string is alphanumeric
int check_alnum(char* str) {
	for (int i=0; i<strlen(str); i++) {
		if (!(isalnum(str[i]))) {
			return 0; 
		}
	}
	return 1; 
}

int echo(char* command_arg) {
	
	char* var = strchr(command_arg, '$');

	if (var ==  NULL) {
		printf("%s\n", command_arg);
	} else {
		char* value = mem_get_value(++var);
		if (strcmp(value, "Variable does not exist")==0) {
			printf("\n");
		} else {
			printf("%s\n", value);
		}
	}
	return 0;
}

int set(char* var, char* value){

	//mem_set_value(var, value);
	set_var(var, value);
	return 0;
}

int print(char* var){
	printf("%s\n", mem_get_value(var)); 
	return 0;
}

int my_ls() {
   	system("ls -1 | sort"); // bash command to sort 
	return 1; 
}

int run(int args_size, char* scripts[]) {
	int errCode = 0;

	errCode = scheduler(args_size, scripts, "run");

	return errCode;
}
