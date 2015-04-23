CC=gcc -fopenmp
C1C=../../bin/c1c
AS=as

all: omprun newdata
omprun:omprun.o omp.o
	${CC} -o $@ $^ -lm

omprun.o:omprun.c
	${CC} -c -o $@ $^

omp.o:omp.s
	${AS} -o $@ $^

omp.s:omp.c1
	${C1C} $^

clean:
	rm -f *.s *.o omprun newdata
