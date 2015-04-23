#include <stdio.h>
extern int data;

void output(){
	printf("%d",data);
}	
void input(){
	scanf("%d",&data);
}

void space(){
	printf(" ");
}

void nextline(){
	printf("\n");
}
