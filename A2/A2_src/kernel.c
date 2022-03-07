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
    int jLengthScore; // job length score
};

#define PCB_NUM 3
#define MAX_USER_INPUT 1000

struct PCB PCBs[PCB_NUM];
int RQ[PCB_NUM];

int current_pid; 

char* RemoveDigits(char* input);
void bubbleSortLength(int array[], int size);
void bubbleSortScore(int array[], int size);
void runStatic(int size);
void runRR(int size);
void runAGING(int size);
int get_current_rq_size();
int process_done(int i);
void run_command(int i, int size);
int rq_isempty(int size);
void clean(struct PCB pcb);
void age_rq(int size);
void mem_clear(char start[], int length);

void PCBs_init(){
    
	for (int i=0; i<PCB_NUM; i++){		
		PCBs[i].pid = -1;
        strcpy(PCBs[i].start, "none");
		PCBs[i].length = 0;
        PCBs[i].pc = -1;
        PCBs[i].jLengthScore = 0;
	} 

    current_pid = 1;

}

void print_all_pcbs(){
    for (int i=0; i<2; i++) { // search all PCBs
        printf("pid: %d\nstart: %s\nlength: %d\npc: %d\njLengthScore: %d\n", 
        PCBs[i].pid, PCBs[i].start, PCBs[i].length, PCBs[i].pc, PCBs[i].jLengthScore);
    }
}

void print_pcb(int pid) {
    for (int i=0; i<PCB_NUM; i++) { // search all PCBs
        if (pid == PCBs[i].pid) {
            printf("pid: %d\nstart: %s\nlength: %d\npc: %d\njLengthScore: %d\n", 
            PCBs[i].pid, PCBs[i].start, PCBs[i].length, PCBs[i].pc, PCBs[i].jLengthScore);
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
            PCBs[i].jLengthScore = length;
            break;
        }
    }
    current_pid++; 
}

void rq_init() {
    for (int i=0; i<PCB_NUM; i++) { 
        RQ[i] = -1;
    }
}

int scheduler(int arg_size, char* scripts[], char policy[]) {

    //print_mem();

    // start clean
    rq_init();
    PCBs_init();

    int i;
    if (arg_size > 2) { // command is "exec", so skip the last argument
        i = arg_size-1;
    } else {
        i = arg_size;
    }

    for (int k=1; k<i; k++) {

        // OPEN FILE
        char line[MAX_USER_INPUT];
	    FILE *p = fopen(scripts[k],"rt");  // the program is in a file

	    if(p == NULL){
            printf("%s\n", "Bad command: File not found");
		    return 1;
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

        // deal with code loading errors
        int errorCode = mem_set_value(var, value);

        if (errorCode) {
            return 1;
        }

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

            // deal with code loading errors
            errorCode = mem_set_value(var, value);

            if (errorCode) {
                return errorCode;
            }
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

    int j = 0; // tracker for RQ (num of elements initially in RQ)
    
    // Populate RQ (default: the order in which they were processed)
    for (int i=0; i<PCB_NUM; i++) { 
        if (PCBs[i].pid != -1) {
            RQ[j] = i;
            j++;
        } 
    }

    //print_rq(3);
    //print_all_pcbs();
    //print_mem();

    //print_rq(get_current_rq_size());

    // run processes
    if (strcmp(policy, "FCFS") == 0) {
        runStatic(arg_size-2);
    } else if (strcmp(policy, "SJF") == 0) {
        bubbleSortLength(RQ, j); // sort RQ
        runStatic(arg_size-2);
    } else if (strcmp(policy, "RR") == 0) {
        runRR(arg_size-2);
    } else if (strcmp(policy, "AGING") == 0) {
        bubbleSortScore(RQ, get_current_rq_size());
        runAGING(arg_size-2);
    } else if (strcmp(policy, "run") == 0) {
        runStatic(arg_size-1);
    }

    return 1;

}

// size is the number of elements in the RQ
void runStatic(int size) {

    //printf("current size is: %d\n", get_current_rq_size());
    //print_rq(get_current_rq_size());

    //print_pcb(1);
    //printf("process is: %d\n", PCBs[RQ[0]].length);

    for (int i=0; i<size; i++) { // complete each job in RQ
        
        // run processes
        while (!process_done(RQ[i])) {
            
            //puts("HERE");
            run_command(i, size); 
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
                run_command(i, size);

                if (!process_done(RQ[i])) {
                    run_command(i, size); 
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

void runAGING(int size) {

    while (!rq_isempty(size)) {

        //print_AGING(get_current_rq_size());

        if (process_done(RQ[0])) {
            clean(PCBs[RQ[0]]);
            RQ[0] = -1;
            bubbleSortScore(RQ, size); // sort RQ
        } else {
            //printf("working on process: %d\n", RQ[0]);
            run_command(0, size);
            // AGE ALL PROCESSES
            age_rq(get_current_rq_size());
            bubbleSortScore(RQ, size); // sort RQ
        }
    }

    //print_mem();
}

// get num of elements in RQ
int get_current_rq_size() {
    int size = 0;
    for (int i=0; i<PCB_NUM; i++) {
        if (RQ[i] != -1) {
            size++;
        }
    }
    return size;
}

// i is index in RQ 
// size is number of elements in RQ
void run_command(int i, int size) {

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

// size is number of elements in RQ
void age_rq(int size) {

    for (int i=1; i<size; i++) {

        if (PCBs[RQ[i]].jLengthScore <= 0) {
            continue;
        } else {
            PCBs[RQ[i]].jLengthScore--;
        }
    }
}

// checks if PCBs[i] is complete
int process_done(int i) { // takes PCB index
    return (PCBs[i].length < PCBs[i].pc);
}

// perform the bubble sort (ascending order) on RQ
void bubbleSortLength(int array[], int size) {

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

// perform the bubble sort (ascending order) on RQ
void bubbleSortScore(int array[], int size) {

    // iterate through each element
    for (int i = 0; i < size - 1; ++i) {
      
        // for each element, compare to every other element
        for (int j = 0; j < size - i - 1; ++j) {
      
            // compare two adjacent elements
            if (PCBs[array[j]].jLengthScore > PCBs[array[j + 1]].jLengthScore) { // swap 
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

/*
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
*/

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

void print_AGING(int size) {
    puts("RQ IS: \n");
    for (int i=0; i<size; i++) {
        printf("element %d: %d, score: %d\n", i, RQ[i], PCBs[RQ[i]].jLengthScore);
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

