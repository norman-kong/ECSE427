#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include "shell.h"
#include "shellmemory.h"

struct PCB {
	int pid;
    char start[100]; // "var" (key) of where first line is stored in mem
    int length; // # of lines the program is
	int pc; // current instruction to execute
};

#define PCB_NUM 100
#define MAX_USER_INPUT 1000

struct PCB PCBs[PCB_NUM];
int RQ[PCB_NUM];

int current_pid; 

char* RemoveDigits(char* input);

void PCBs_init(){
    
	for (int i=0; i<PCB_NUM; i++){		
		PCBs[i].pid = -1;
        strcpy(PCBs[i].start, "none");
		PCBs[i].length = 0;
        PCBs[i].pc = -1;
	} 

    current_pid = 1;

}

void print_all_pcbs(){
    for (int i=0; i<2; i++) { // search all PCBs
        printf("pid: %d\nstart: %s\nlength: %d\npc: %d\n", 
        PCBs[i].pid, PCBs[i].start, PCBs[i].length, PCBs[i].pc);
    }
}

void print_pcb(int pid) {
    for (int i=0; i<PCB_NUM; i++) { // search all PCBs
        if (pid == PCBs[i].pid) {
            printf("pid: %d\nstart: %s\nlength: %d\npc: %d\n", 
            PCBs[i].pid, PCBs[i].start, PCBs[i].length, PCBs[i].pc);
            break;
        }
    }
}

void create_pcb(char *start, int length) {

    for (int i=0; i<PCB_NUM; i++) { 
        if (PCBs[i].pid == -1) { // find open slot
            PCBs[i].pid = current_pid;
            strcpy(PCBs[i].start, start); 
            PCBs[i].length = length;
            PCBs[i].pc = 1;
            break;
        }
    }
    current_pid++; 
}

void init_rq() {
    for (int i=0; i<PCB_NUM; i++) { 
        RQ[i] = -1;
    }
}

void scheduler(int arg_size, char* scripts[], char policy[]) {

    int i;
    if (arg_size > 2) { // command is "exec", so skip the last argument
        i = arg_size-1;
    } else {
        i = arg_size;
    }

    //print_mem();

    for (int k=1; k<i; k++) {

        // OPEN FILE
        char line[MAX_USER_INPUT];
	    FILE *p = fopen(scripts[k],"rt");  // the program is in a file

	    if(p == NULL){
		    return badcommandFileDoesNotExist();
	    }

        // for PCB
        int lineNumInt = 1;
        char lineNumStr[5];
        sprintf(lineNumStr, "%d", lineNumInt);

        // get first line
        fgets(line,999,p);

        // STORE FIRST LINE
        char var[100]; // var format: filenameLine27
        strcpy(var, scripts[k]);
        strcat(var, "Line");
        strcat(var, lineNumStr); 
        char *value = line;
        mem_set_value(var, value);

        char start[MAX_USER_INPUT]; // holds key of first line
        strcpy(start, var);
            
        memset(line, 0, sizeof(line));
        memset(var, 0, sizeof(var));

        fgets(line,999,p); // get next line
        lineNumInt++;
        sprintf(lineNumStr, "%d", lineNumInt);

        // STORE REST OF THE LINES
        while(1){
            
            strcpy(var, scripts[k]);
            strcat(var, "Line");
            strcat(var, lineNumStr); 
            char *value = line;
            mem_set_value(var, value);

            memset(line, 0, sizeof(line));
            memset(var, 0, sizeof(var));

            if(feof(p)){
                break;
            }

            fgets(line,999,p);
            lineNumInt++;
            sprintf(lineNumStr, "%d", lineNumInt);
        }

        fclose(p);

        create_pcb(start, lineNumInt);
        //print_mem();
    } 

    //print_mem();

    int j = 0; // tracker for RQ
    
    // Populate RQ (default: the order in which they were processed)
    for (int i=0; i<PCB_NUM; i++) { 
            if (PCBs[i].pid != -1) {
                RQ[j] = i;
                j++;
            }
    }

    // run processes
    if (strcmp(policy, "FCFS") == 0) {
        runStatic(arg_size-2);
    } else if (strcmp(policy, "SJF") == 0) {
        bubbleSort(RQ, j); // sort RQ
        runStatic(arg_size-2);
    } else if (strcmp(policy, "RR") == 0) {
        runRR(arg_size-2);
    }

}

// size is the number of elements in the RQ
void runStatic(int size) {
    for (int i=0; i<size; i++) { // complete each job in RQ
        
        // run processes
        while (!process_done(RQ[i])) {
            run_command(i); // run 1 command
        }
        //print_mem();
        clean(PCBs[RQ[i]]);
    }
    //print_mem();
}

// size is the number of elements in the RQ
void runRR(int size) {

    while (!rq_isempty(size)) {
        
        for (int i=0; i<size; i++) { // iterate through all processes in RQ

            //printf("looking at process: %d\n", RQ[i]);

            if (!process_done(RQ[i])) {
                //printf("working on process: %s\n", PCBs[RQ[i]].start);
                run_command(i);

                if (!process_done(RQ[i])) {
                    run_command(i); 
                } else {
                    clean(PCBs[RQ[i]]);
                    RQ[i] = -1;
                }

            } else {
                clean(PCBs[RQ[i]]);
                RQ[i] = -1;
            }
        }
    }		
    //print_mem();
}


// i is index in RQ 
void run_command(int i) {

    // CONSTRUCT KEY 
    char key[100];
            
    // Remove line number
    char *name = strtok(PCBs[RQ[i]].start, "Line");
    strcpy(key, name);
    strcat(key, "Line");

    // get key value without the line num (so we can replace with PC)
    //sprintf(key, RemoveDigits(PCBs[RQ[i]].start));

    // get PC num and add to key
    char pc_num[5];
    sprintf(pc_num, "%d", PCBs[RQ[i]].pc);
    strcat(key, pc_num); 
                
    //printf("KEY IS: %s\n", key);
    //printf("command is: %s\n", mem_get_value(key));
    parseInput(mem_get_value(key)); // sends line to interpreter
    PCBs[RQ[i]].pc++; // go to next line
}

// checks if PCBs[i] is complete
int process_done(int i) { // takes PCB index
    return (PCBs[i].length < PCBs[i].pc);
}

// perform the bubble sort (ascending order) on RQ
void bubbleSort(int array[], int size) {

    // iterate through each element
    for (int i = 0; i < size - 1; ++i) {
      
        // for each element, compare to every other element
        for (int j = 0; j < size - i - 1; ++j) {
      
            // compare two adjacent elements
            if (PCBs[array[j]].length > PCBs[array[j + 1]].length) { // swap 
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

// helper
char* RemoveDigits(char* input) {
    char* dest = input;
    char* src = input;

    while(*src)
    {
        if (isdigit(*src)) { src++; continue; }
        *dest++ = *src++;
    }
    *dest = '\0';
    return input;
}

void clean(struct PCB pcb) {

    char start[100];
    strcpy(start, pcb.start);
    
    // add "Line"
    strcat(start, "Line");

    // add 1 to end
    char num[5];
    sprintf(num, "%d", 1);
    strcat(start, num); 

    mem_clear(start, pcb.length); 
}

// size is number of elements in RQ
void print_rq(int size) {
    puts("RQ IS: \n");
    for (int i=0; i<size; i++) {
        printf("element %d: %d\n", i, RQ[i]);
    }
}

void print_int_array(int a[], int size) {
    for (int i=0; i<size; i++) {
        printf("element %d: %d\n", i, a[i]);
    }
}

// helper 
int rq_isempty(int size) {
    for (int i=0; i<size; i++) {
        if (RQ[i] != -1) {
            return 0;
        }
    }
    return 1;
}

