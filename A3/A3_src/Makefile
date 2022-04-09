mysh: shell.c interpreter.c shellmemory.c kernel.c
	gcc -c shell.c interpreter.c shellmemory.c kernel.c
	gcc -o mysh shell.o interpreter.o shellmemory.o kernel.o

clean: 
	rm mysh; rm *.o
