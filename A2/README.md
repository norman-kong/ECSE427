# Notes regarding Assignment 2

- Do not put spaces after your commands; this is undefined behaviour. 
- The scheduler is programmed to sort the RQ by job length score after each time slice for AGING. That is, after every instruction, the scheduler will sort the RQ in ascending order (again, by comparing each PCB's job length score). This was a design decision and is justified/allowed; see Ed post #405 (https://edstem.org/us/courses/16123/discussion/1247019). 
