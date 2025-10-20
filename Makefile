mysh: mysh.o mystring.o myheap.o jobs.o
	gcc mysh.o mystring.o myheap.o jobs.o get.o -o mysh

test: test.o mystring.o myheap.o jobs.o
	gcc test.o mystring.o myheap.o jobs.o -o test

test.o: test.c
	gcc -c test.c

mysh.o: mysh.c mystring.h jobs.h
	gcc -c mysh.c

mystring.o: mystring.c mystring.h
	gcc -c mystring.c

myheap.o: myheap.c myheap.h
	gcc -c myheap.c

jobs.o: jobs.c jobs.h myheap.h
	gcc -c jobs.c

get.o: get.c get.h
	gcc -c get.c

clean:
	/usr/bin/rm -f *.o mysh

all: clean mysh
