#include <stdio.h>
#include <omp.h>

extern const int f[];
extern int c[10];

extern void testfunc();

void ex(){
	c[7]=-987;
	int t=omp_get_thread_num();
	printf("hello world from thread %d\n",t);
}

int main(){
	int i;
	for(i=0;i<16;i++)printf("%d ",f[i]);
	printf("\n");
	for(i=0;i<10;i++)c[i]=i;
	printf("\n");
	testfunc();
	for(i=0;i<10;i++)printf("%d ",c[i]);
	printf("function test ok.\n");
	return 0;
}
