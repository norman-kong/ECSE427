#include<stdlib.h>
#include<string.h>
#include<stdio.h>

struct memory_struct{
	char *var;
	char *value;
};

#define FRAME_SIZE 3 // each page/frame is 3 lines of code
#define NUM_FRAMES (FRAME_MEM_SIZE/3)
#define FRAME_STORE_SIZE (FRAME_SIZE*NUM_FRAMES)
#define VAR_STORE_SIZE VAR_MEM_SIZE // last VAR_STORE_SIZE lines of the memory are used for tracking variables
#define MEM_SIZE (FRAME_STORE_SIZE + VAR_STORE_SIZE)

//int mem_size = FRAME_MEM_SIZE/3;

int frames[NUM_FRAMES]; // 0 is free, 1 is taken
struct memory_struct shellmemory[MEM_SIZE];

void resetmem() {
	for (int i= (MEM_SIZE - VAR_STORE_SIZE) ; i<MEM_SIZE; i++) {
		//puts("RESET_MEM");
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

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
	
	//puts("MEM_INIT");
	int i;
	for (i=0; i<MEM_SIZE; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
	for (i=0; i<NUM_FRAMES; i++) {
		frames[i] = 0;
	}
}

// finds the next available frame and sets it to unavailable
int find_available_frame() {

	for (int i=0; i<NUM_FRAMES; i++) {
		if (frames[i] == 0) {
			//printf("now using frame: %d\n", i);
			frames[i] = 1;
			return i;
		}
	}
	//print_frame_store();
	//puts("NO FRAMES AVAILABLE");
	return -1;
}

void print_frame_store() {
	puts("PRINTING FRAME STORE");
	for (int i=0; i<NUM_FRAMES; i++) {
		printf("frame number %d is %d\n", i, frames[i]);
	}
}
 
// sets an entry with var_in, value_in, in slot spot
void mem_set_val2(char *key, char *val, int spot) {
	//printf("ADDING KEY: %s to spot: %d\n", key, spot);

	shellmemory[spot].var = strdup(key);
	shellmemory[spot].value = strdup(val);
}

// clears [length] lines in memory starting from [start]
void mem_clear(char start[], int length) {

	//puts("MEM_CLEAR");
	for (int i=0; i<MEM_SIZE; i++) {		
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

void clear_entry(int i) {
	shellmemory[i].var = "none";
	shellmemory[i].value = "none";
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

// set var in variable store 
int set_var(char *var_in, char *value_in) {
	
	int i;

	for (i=MEM_SIZE-VAR_STORE_SIZE; i<MEM_SIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return 0;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=MEM_SIZE-VAR_STORE_SIZE; i<MEM_SIZE; i++){
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
	int frame = 0;
	for (int i = 0; i<FRAME_STORE_SIZE; i++){

		if (i % 3 == 0) {
			printf("FRAME NUMBER: %d\n", frame);
			frame++;
		}
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

// returns 1 if key is in mem, 0 otherwise 
int in_mem(char *var_in) {
	for (int i=0; i<MEM_SIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return 1;
		} 
	}
	return 0;
}

// gets the value at index i
char *get_val_at_index(int index) {
	return shellmemory[index].value;
}
