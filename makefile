make compile:
	gcc -Wall -pedantic-errors shell.c -o shell

make run:
	./shell

make clean:
	rm -f shell
