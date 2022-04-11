mysh: shell.c interpreter.c shellmemory.c kernel.c
	gcc -D FRAME_MEM_SIZE=$(framesize) -D VAR_MEM_SIZE=$(varmemsize) -c shell.c interpreter.c shellmemory.c kernel.c
	gcc -o mysh shell.o interpreter.o shellmemory.o kernel.o
	
clean: 
	rm mysh; rm *.o
