#include <stdio.h>
#include <omp.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#define MAXN 1000000

extern void c1run();
extern int a[MAXN],b[MAXN],c[MAXN];
extern int sum;

extern int n;

char filename[]="data.in";

pthread_mutex_t mutex;

struct timeval start,finish;

void read(){
	FILE *fp;
	int i;
	fp=fopen(filename,"rt");
	fscanf(fp,"%d",&n);
	for(i=0;i<n;i++){
		fscanf(fp,"%d %d %d",a+i,b+i,c+i);
	}
	fclose(fp);
}

void para_start(){
	gettimeofday(&start,NULL);
}

void para_finish(){
	gettimeofday(&finish,NULL);
}


void para(){
	int lsum=0;
	int i;
	int low,high,ave,res;
	int tid,tnum;
	tid=omp_get_thread_num();
	tnum=omp_get_num_threads();
	ave=n/tnum;
	res=n%tnum;
	if(tid<res){
		low=(ave+1)*tid;
		high=(ave+1)*(tid+1);
	}else{
		low=ave*tid+res;
		high=ave*(tid+1)+res;
	}
	printf("thread id %d calculate sum from %d to %d\n",tid,low,high-1);
	for(i=low;i<high;i++){
		lsum+=(int)(10.0*sin(a[i]*b[i])+(int)pow(b[i],c[i])%13+5*tan(a[i]*c[i]));
	}
	printf("thread id %d :thread sum is %d \n",tid,lsum);
	__sync_fetch_and_add(&sum,lsum);
}

void output(){
	int t_in_usec;
	t_in_usec=(finish.tv_sec-start.tv_sec)*1000000+(finish.tv_usec-start.tv_usec);
	printf("sum = %d\n",sum);
	printf("time used:%d usec\n",t_in_usec);
}

int main(){
	c1run();
}
