#include<stdlib.h>
#include<string.h>
#include<stdio.h>

struct memory_struct{
	char *var;
	char *value;
};

#define MEM_SIZE 1000

struct memory_struct shellmemory[MEM_SIZE];

// Helper functions
int match(char *model, char *var) {
	int i, len=strlen(var), matchCount=0;
	for(i=0;i<len;i++)
		if (*(model+i) == *(var+i)) matchCount++;
	if (matchCount == len)
		return 1;
	else
		return 0;
}

char *extract(char *model) {
	char token='=';    // look for this to find value
	char value[1000];  // stores the extract value
	int i,j, len=strlen(model);
	for(i=0;i<len && *(model+i)!=token;i++); // loop till we get there
	// extract the value
	for(i=i+1,j=0;i<len;i++,j++) value[j]=*(model+i);
	value[j]='\0';
	return strdup(value);
}


// Shell memory functions

void mem_init(){

	int i;
	for (i=0; i<MEM_SIZE; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

// clears [length] lines in memory starting from [start]
void mem_clear(char start[], int length) {
	for (int i=0; i<1000; i++) {		
		if (strcmp(shellmemory[i].var, start) == 0) { // find first line
			shellmemory[i].var = "none";
			shellmemory[i].value = "none";

			for (int j=i+1; j<i+length; j++) { // clear rest of program			
				shellmemory[j].var = "none";
				shellmemory[j].value = "none";
			}
			break;
		}
	}
}

// Set key value pair
int mem_set_value(char *var_in, char *value_in) {
	
	int i;

	for (i=0; i<MEM_SIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return 0;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<MEM_SIZE; i++){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			return 0;
		} 
	}

	printf("%s\n", "Code loading error");
	return 1;

}

void print_mem() {
	puts("MEMORY IS CURRENTLY: ");
	for (int i = 0; i<MEM_SIZE; i++){
		printf("key is: %s\n", shellmemory[i].var);
		printf("value is: %s\n", shellmemory[i].value);
	}
	puts("FINISHED PRINTING MEMORY\n");
	return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
	int i;

	for (i=0; i<MEM_SIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return strdup(shellmemory[i].value);
		} 
	}
	return "Variable does not exist";

}