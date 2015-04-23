C1C=../../bin/c1c
AS=as
CC=gcc -fopenmp -O3
ICC=icc -O3 -xHost -fast -ipo
all:quickpow quicksort newdata cquicksort iccquicksort

cquicksort:cquicksort.o stdio.o
	${CC} -o $@ $^

iccquicksort:cquicksort.c stdio.c
	${ICC} -o $@ $^

%:%.o stdio.o
	${CC} -o $@ $^

%.o:%.s
	${AS} -o $@ $^

%.s:%.c1
	${C1C} $^

cquicksort.o:cquicksort.c
	${CC} -c $^

stdio.o:stdio.c
	${CC} -c $^

newdata:newdata.c

.PHONY:clean
clean:
	rm -rf *.o *.s quickpow quicksort cquicksort newdata
