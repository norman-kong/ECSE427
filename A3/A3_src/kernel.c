#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include <dirent.h> // for DIR 

#include "shell.h"
#include "shellmemory.h"

#define FRAME_SIZE 3 // each page/frame is 3 lines of code

struct PCB {
	int pid;
    char start[100]; // "var" (key) of where first line is stored in mem
    int length; // # of lines the program is
	int pc; // current instruction to execute
    int jLengthScore; // job length score
    int pagetable[10]; 
};

#define PCB_NUM 4   // CHANGE THIS BEFORE SUBMISSION
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
void run_command(int i);
int rq_isempty(int size);
void clean(struct PCB pcb);
void age_rq(int size);
void mem_clear(char start[], int length);
void mem_clear_pcb(struct PCB pcb);
void mem_set_val2(char *key, char *val, int spot);
int find_available_frame();
char *get_val_at_index(int index);

void PCBs_init(){
    
	for (int i=0; i<PCB_NUM; i++){		
		PCBs[i].pid = -1;
        strcpy(PCBs[i].start, "none");
		PCBs[i].length = 0;
        PCBs[i].pc = -1;
        PCBs[i].jLengthScore = 0;
        for (int k=0; k<10; k++) {
            PCBs[i].pagetable[k] = -1;
        }

	} 

    current_pid = 1;

}

void print_PCBs_pid() {
    puts("starting");
    for (int i=0; i<PCB_NUM; i++){		
		printf("PID IS: %d\n", PCBs[i].pid);
	} 
    puts("done");
}

// prints all valid PCBs
void print_all_pcbs(){
    for (int i=0; i<PCB_NUM; i++) { 

        if (PCBs[i].pid != -1) {
            printf("pid: %d\nstart: %s\nlength: %d\npc: %d\njLengthScore: %d\n", 
            PCBs[i].pid, PCBs[i].start, PCBs[i].length, PCBs[i].pc, PCBs[i].jLengthScore);
            puts("pagetable: ");
            
            for (int k=0; k<3; k++) {
                printf("pagetable slot: %d is %d\n", k, PCBs[i].pagetable[k]);
            }
        }

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

void create_pcb(char *start, int length, int pagetable[]) {
    
    for (int i=0; i<PCB_NUM; i++) { 
        if (PCBs[i].pid == -1) { // find open slot
            PCBs[i].pid = current_pid;
            strcpy(PCBs[i].start, start); 
            PCBs[i].length = length;
            PCBs[i].pc = 1;
            PCBs[i].jLengthScore = length;

            for (int k=0; k<10; k++) { // ASSUMPTION: PAGE TABLES ARE OF SIZE 10
                PCBs[i].pagetable[k] = pagetable[k];
            }
        
            //memcpy(PCBs[i].pagetable, pagetable, sizeof(pagetable)); didn't work for some reason
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

    // start clean
    rq_init();
    PCBs_init();

    //print_frame_store();

    //print_mem();

    int i;
    if (arg_size > 2) { // command is "exec", so skip the last argument
        i = arg_size-1;
    } else {
        i = arg_size;
    }

    //for (int k=1; k<i; k++) {

    struct dirent *pDirent;
    DIR *pDir;
    pDir = opendir("backing_store/");
    if (pDir == NULL) {
        printf ("Cannot open directory '%s'\n", "backing_store/");
        return 1;
    }

    /* CODE LOADING */
    // Process each entry
    while ((pDirent = readdir(pDir)) != NULL) {
        if (strcmp(pDirent->d_name, ".") != 0) {
            if (strcmp(pDirent->d_name, "..") != 0) {

                //printf("LOADING CODE OF FILE: %s\n", pDirent->d_name);

                // OPEN FILE
                char path[100] = "backing_store/";
                strcat(path, pDirent->d_name);
	            FILE *p = fopen(path,"rt");  // the program is in a file
	                
                if(p == NULL){
                    printf("%s\n", "Bad command: File not found");
	                return 1;
	            }

                // file is empty 
                if(feof(p)){
                    int frame = find_available_frame(); 
                    for (int i=0; i<3; i++) {
                        mem_set_val2("-1","-1",frame+i); 
                    }
                }

                // first line num 
                int lineNumInt = 1;
                char lineNumStr[5];
                sprintf(lineNumStr, "%d", lineNumInt);

                // to get "start"
                int first_iteration = 1;
                char start[MAX_USER_INPUT]; // holds key of first line

                // page table
                int pagetable[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
                int counter = 0; 

                //while (!feof(p)) { // there is at least 1 line 

                int linesRemain;
                int pages_loaded = 0;

                if (feof(p)) {
                    linesRemain = 0;
                } else {
                    linesRemain = 1;
                }

                while (linesRemain && pages_loaded < 2) { 

                    //printf("LOADING CODE OF FILE: %s\n", pDirent->d_name);

                    int frame = find_available_frame();
                    pages_loaded++;

                    //printf("FRAME IS: %d\n", frame);

                    for (int j=0; j<3; j++) {

                        if (feof(p)) { // we have reached the end of the file, fill the rest of the frame with whatever
                            linesRemain = 0;    
                            for (int k=j + (frame*3); k<(frame+1)*3; k++) {
                                mem_set_val2("-1", "-1", k);
                            }
                            break;
                                
                        } else {
                                
                            char line[MAX_USER_INPUT];
                            fgets(line,999,p);

                            char var[100]; // var format: filename-Line27
                            strcpy(var, pDirent->d_name);
                            strcat(var, "-Line");
                            strcat(var, lineNumStr); 
                            char *value = line;

                            // just saving start value
                            if (first_iteration) {
                                first_iteration = 0;
                                strcpy(start, var);
                            }

                            mem_set_val2(var, value, frame*FRAME_SIZE+j);

                            // update line num
                            lineNumInt++;
                            sprintf(lineNumStr, "%d", lineNumInt);
                        }
                        
                    } 
                    //printf("FRAME IS: %d\n", frame);
                    //printf("COUNTER IS: %d\n", counter);
                    pagetable[counter] = frame;
                    counter++;
                }

                while (!(feof(p))) {
                    char line[MAX_USER_INPUT];
                    fgets(line,999,p);
                    lineNumInt++;
                }


                fclose(p);
        
                create_pcb(start, lineNumInt-1, pagetable);
                //print_PCBs_pid();
                //print_all_pcbs();
            }
        }
    }

    //print_all_pcbs();
    //print_mem(); 
    //print_frame_store();
    //exit(1);
    //print_frame_store();
    //exit(1);

    //} 

    // running code

    int j = 0; // tracker for RQ (num of elements initially in RQ)
    
    // Populate RQ (default: the order in which they were processed)
    for (int i=0; i<PCB_NUM; i++) { 
        if (PCBs[i].pid != -1) {
            RQ[j] = i;
            j++;
        } 
    }

    //print_all_pcbs();
    //exit(1);

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

    //print_all_pcbs();

    return 1;

}

// size is the number of elements in the RQ
void runStatic(int size) {

    //printf("current size is: %d\n", get_current_rq_size());
    //print_rq(get_current_rq_size());

    //print_pcb(0);
    //print_all_pcbs();
    //printf("process is: %d\n", PCBs[RQ[0]].length);

    for (int i=0; i<size; i++) { // complete each job in RQ

        // run processes
        while (!process_done(RQ[i])) {
            run_command(i); 
        }
        //print_mem();
        clean(PCBs[RQ[i]]);
    }
    //print_mem();
}

// size is the number of elements in the RQ
void runRR(int size) {

    while (!rq_isempty(size)) {

        int i = 0;
        
        //for (int i=0; i<size; i++) { // iterate through all processes in RQ

            //printf("looking at process: %d\n", RQ[i]);

            //if (RQ[i] != -1) {
                
                if (!process_done(RQ[i])) {
                    //printf("working on process: %s\n", PCBs[RQ[i]].start);
                    run_command(i);
                    run_command(i);

                    //if (!process_done(RQ[i])) { 
                        //run_command(i); 
                        push_to_end(i, PCB_NUM);
                    //} else {
                        //clean(PCBs[RQ[i]]);
                       // RQ[i] = -1;
                        //fix_rq();
                    //}

                } else {
                    //clean(PCBs[RQ[i]]);
                    RQ[i] = -1;
                    fix_rq();
                }

            //} 

        //}
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
            run_command(0);
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

// i is the index of the process you want to push to the end
void push_to_end(int i, int size){
    
    int new[size]; 

    for (int w=0;w<size;w++) {
        new[w] = -1;
    }

    // copying elements before i
    for (int k=0; k<i; k++) {
        new[k] = RQ[k];
    }

    // copying elements after i
    for (int k=i; k<size; k++) {
        if (RQ[k+1] == -1) {
            for (int g=k; g<size; g++) {
                new[g] = -1;
            }
            break;
        }
        new[k] = RQ[k+1];
    }
/*
    for (int d=0;d<size;d++) {
        printf("INTERMEDIATE IS: %d\n", new[d]);
    }
*/
    // placing the element at the back
    for (int n=0; n<size; n++) {
        if (new[n] == -1) {
            new[n] = RQ[i];
            break;
        }
    }

    // update RQ
    for (int m=0; m<size; m++) {
        RQ[m] = new[m];
    }
}

void print_rq() {
    puts("PRINTING RQ");
	for (int i=0; i<PCB_NUM; i++) {
        printf("result is: %d\n", RQ[i]);
        //if (RQ[i] != -1) {
        //    puts(PCBs[RQ[i]].start);
        //}
	}
    puts("DONE PRINTING RQ");
}

// i is index in RQ 
void run_command(int i) {

    char key[100]; // CONSTRUCT KEY 
    char path[100] = "backing_store/";

    //print_all_pcbs();
    //printf("START IS: %s\n", PCBs[RQ[i]].start);

    /*
    if (strcmp(PCBs[RQ[i]].start, "-1") == 0) {
        PCBs[RQ[i]].pc++; // go to next line
        return;
    }
    */

    if (PCBs[RQ[i]].pc > PCBs[RQ[i]].length) {
        return;
    }

    // Remove line number
    //printf("start is: %s\n", PCBs[RQ[i]].start);

    //puts("BEFORE /////////////////////////////////////////////////////");
    //print_all_pcbs();
    char temp[100];
    strcpy(temp, PCBs[RQ[i]].start);
    char *name = strtok(temp, "-");
    
    //puts("AFTER /////////////////////////////////////////////////////");
    //print_all_pcbs();
    //char *name = PCBs[RQ[i]].start;
    strcpy(key, name);
    strcat(path, name);
    strcat(key, "-Line");

    // get PC num and add to key
    char pc_num[5];
    sprintf(pc_num, "%d", PCBs[RQ[i]].pc);
    strcat(key, pc_num); 

    //print_mem();
    //printf("looking for key: %s\n", key);
    //printf("PC is: %d\n", PCBs[RQ[i]].pc);
    //printf("START IS: %s\n", PCBs[RQ[i]].start);
    //printf("NAME IS: %s\n", name);PCBs[RQ[i]].pc++; // go to next line
    
    // line is not found, load next page 
    if (!(in_mem(key))) {

        //printf("COULDNT FIND KEY: %s\n", key);

        //puts("BEFORE");
        //print_mem();

        // open file from backing store 
	    FILE *p = fopen(path, "rt");  // the program is in a file

        //printf("working on file: %s\n", path);
	                
        if (p == NULL){
            //printf("file is: %s", path);
            printf("%s\n", "Bad command: File not found");
            return 1;
        }

        int frame = find_available_frame();
        
        // LOUD PAGE FAULT
        if (frame == -1) {

            //print_mem();
            int frame_to_replace = 0; // replace 1st page - TO CHANGE IN NEXT PART
            puts("Page fault! Victim page contents:");
            for (int g=0; g<FRAME_SIZE; g++) {
                printf("%s\n", get_val_at_index(frame_to_replace*3 + g));
            }
            puts("End of victim page contents.");
            
            frame = 0; // TO CHANGE IN NEXT PART

            // TODO: UPDATE PAGE TABLE OF VICTIM PROGRAM (actually might not be necessary)
        }

        // load needed page
        for (int j=0; j<3; j++) {

            if (feof(p)) { // we have reached the end of the file, fill the rest of the frame with whatever   
                for (int k=j + (frame*3); k<(frame+1)*3; k++) {
                    //puts("HERE");
                    mem_set_val2("-1", "-1", k);
                }
                break;
                                
            } else {

                // find line we were at
                char line[MAX_USER_INPUT];
                int lineNum = 1;
                while (lineNum != PCBs[RQ[i]].pc +1) {
                    fgets(line,999,p);
                    lineNum++;
                } 
                
                char key[100]; // CONSTRUCT KEY 
                char temp[100];
                strcpy(temp, PCBs[RQ[i]].start);
                //puts("START IS: ");
                //puts(PCBs[RQ[i]].start);
                char *name = strtok(temp, "-");
                strcpy(key, name);
                strcat(key, "-Line");

                // get PC num and add to key
                char pc_num[5];
                sprintf(pc_num, "%d", PCBs[RQ[i]].pc+j);
                strcat(key, pc_num);              
                
                //puts(key);
                //puts(line);
                mem_set_val2(key, line, frame*FRAME_SIZE+j);
            }                
        } 

        //print_all_pcbs();

        // update page table after adding a page to mem
        for (int f=0; f<10; f++) {
            if (PCBs[RQ[i]].pagetable[f] == -1) {
                PCBs[RQ[i]].pagetable[f] = frame;
                break;
            }
        }

        //puts("AFTER");
        //print_mem();
        //print_rq();
        push_to_end(i, PCB_NUM);
        //print_rq();
        return;
    }

    
    //print_all_pcbs();
    //printf("key is: %s\n", key);

    // dealing with one liners
    int errorCode = 0;					// zero means no error, default
	char* commands = strtok(mem_get_value(key), ";"); 
    //printf("key is: %s\n", key);

    /*
    if (strcmp(commands, "-1") == 0) {
        PCBs[RQ[i]].pc++; // go to next line
        return;
    }*/

	while (commands) {
        //printf("command is: %s\n", commands);
		errorCode = parseInput(commands);
		if (errorCode == -1) exit(99);	// ignore all other errors
		memset(commands, 0, sizeof(commands));
		commands = strtok(NULL, ";"); // get next command
	}

    //parseInput(mem_get_value(key)); // sends line to interpreter
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

void fix_rq() {

    int new[4];
    int counter = 0;

    for (int i=0; i<4; i++) {
        if (RQ[i] == -1) {
            continue;
        } else {
            new[counter] = RQ[i];
            counter++; 
        }
    }

    for (int k=counter; k<4; k++) {
        new[k] = -1;
    }

    for (int j=0; j<4; j++) {
        RQ[j] = new[j];
    }
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
    //strcat(start, "-Line");

    // add 1 to end
    //char num[5];
    //sprintf(num, "%d", 1);
    //strcat(start, num); 
    //printf("CLEANING PCB with start: %s\n", start);

    //mem_clear(start, pcb.length); 
    mem_clear_pcb(pcb);
}

void mem_clear_pcb(struct PCB pcb) {
	for (int i=0; i<10; i++) {
		if (pcb.pagetable[i] != -1) {
			for (int k=0; k<3; k++) {
                clear_entry(pcb.pagetable[i]*3+k);
			}
		}
	}
}

/*
// size is number of elements in RQ
void print_rq(int size) {
    puts("RQ IS: \n");
    for (int i=0; i<size; i++) {
        printf("element %d: %d\n", i, RQ[i]);
    }
}
*/

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
