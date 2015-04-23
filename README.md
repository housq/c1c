C1 Compiler with OpenMP directives
==========================

1. Introduction

This project contains two point:
	a. C-linkable x86 asm code generation for C1, generate asm in gnu-asm format
	b. openmp auto parallel for C1


2. Source Tree
This project includes the following files and 
directories:
-----------------------------------------------------------------
README		this file
Makefile	gnu makefile
include		directory storing c header files
config		directory storing lex and yacc definitions
src		directory storing source files
doc		directory storing documents
test		directory storing c1 test codes
bin		directory storing execution/objective and scripts files
-----------------------------------------------------------------

3. Dependency
GNU OpenMP Library should be installed
For Ubuntu/Debian
   $ apt-get install libgomp1

4. Building c1 compiler
   $ make
This will generate a executable file name "c1c" in bin folder
Run /path/to/c1c *.c1 to generate a .s file

(Clean this project)
   $ make clean

5. About test files
	test-----------
		codegen: basic dynamic&static memory alloc, extern link and openmp test
		runtest: a C1 stdio library written in C with two test program, 
				quickpow &quicksort
				a C version quicksort compiled in gcc -O3 is also provided
		openmp:  openmp parallel math calculation,generate 3 array a, b, c of 
			the same size and calculate the sum of 
			(int)(10.0*sin(a[i]*b[i])+(int)pow(b[i],c[i])%13+5*tan(a[i]*c[i]))


6. test/codegen
Make all
   $ make
Run
   $ ./test
You will see some array dump and openmp hello world from different thread

7. test/runtest
Make all
   $ make

Run
   $ ./quickpow
input 3 number a,b,c(!=0)
get a result of a^b%c

Run
   $ ./quicksort
input 1 number n and n int32 numbers
get n sorted numbers

Run
   $ ./cquicksort
the same as quicksort ,this time the code is compiled in gcc -O3

Run
   $ ./newdata [number]
such as ./newdata 1000000 ,(100000000 is the max )you can get a formated
input in file data.in

Then Run
   $ ./quicksort <data.in >data.out
or
   $ ./cquicksort <data.in >data.out
sorted result will be in data.out

Run
   $ ./speedrace
to test runtime of quicksort and cquicksort
suggested number of data is 10^7

8.test/openmp
Make all
   $ make

Run
   $ ./newdata [1-1000000]
to generate a data.in file

Run
   $ ./omprun
to run omp parallel calculating sum of
	(int)(10.0*sin(a[i]*b[i])+(int)pow(b[i],c[i])%13+5*tan(a[i]*c[i]))

Use
   $ export OMP_NUM_THREADS=n
n is number of threads used ,default n is equal to cpu number

   
========
Hou Siqing ( housq@mail.ustc.edu.cn )
School of Computer Science and Technology
University of Science and Technology of China
Hefei, Anhui, P.R.China

Jan.13, 2015

