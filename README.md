# Custom Operating System 

This repo contains the code for an operating system (written in C) that runs in the shell. It is divided as follows: 

A1: Program the base of an operating system in C that runs in the shell.  

A2: Extend the OS to support running concurrent processes. Implemented the scheduling infrastructure (code loading, PCB data structure, ready queue data structure, scheduler logic), implement functionality for the OS to run processes according to various scheduling policies (SJF, RR, SJF with job aging). 

A3: Added paging infrastructure (backign store, partitioning shell memory, code loading, page table, scaffolding), designed and implemented demand paging (setting shell memory size at compile time, code loading, handling page faults), implemented the LRU replacement policy in demand paging. We allow programs larger than the shell memory size to be run by our OS; to do so we split the program into pages; only the necessary pages will be loaded into memory and old pages will be switched out when the shell memory gets full. 

This was completed as an assignment for the course ECSE 427 (Operating Systems) at McGill University (W2022). The topics of this course include: file system organization, disk and CPU scheduling, virtual memory management, concurrent processing and distributed systems. Assignment PDFs cannot be posted publicly but details can be discussed privately. 
