test:test.o allocate.o
	gcc -fopenmp -o test $^
test.o:test.c
	gcc -fopenmp -c test.c
allocate.o:allocate.s
	as -o allocate.o allocate.s
allocate.s:allocate.c1
	../../bin/c1c allocate.c1

clean:
	rm -f *.s *.o test


