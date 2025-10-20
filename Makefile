mysh: mysh.o mystring.o myheap.o getjob.o runjob.o
	gcc mysh.o mystring.o myheap.o getjob.o runjob.o -o mysh

mysh.o: mysh.c getjob.h runjob.h jobs.h
	gcc -c mysh.c

mystring.o: mystring.c mystring.h
	gcc -c mystring.c

myheap.o: myheap.c myheap.h
	gcc -c myheap.c

getjob.o: getjob.c getjob.h jobs.h myheap.h mystring.h
	gcc -c getjob.c

runjob.o: runjob.c runjob.h jobs.h
	gcc -c runjob.c

clean:
	/usr/bin/rm -f *.o mysh

all: clean mysh
