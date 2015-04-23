#include <stdio.h>
#include <stdlib.h>
#define MAXN 1000000

int n;

char filename[]="data.in";

int main(int argc,char **argv){
	FILE *fp;
	int	i;
	if(argc<2){
		fprintf(stderr,"usage: %s [positive number<=1000000]\n",argv[0]);
		exit(1);
	}
	sscanf(argv[1],"%d",&n);
	srand(time(NULL));
	fp=fopen(filename,"wt");
	fprintf(fp,"%d\n",n);
	for(i=0;i<n;i++){
		fprintf(fp,"%d %d %d\n",rand()%10,rand()%10,rand()%10);
	}
	fclose(fp);
	return 0;
}
