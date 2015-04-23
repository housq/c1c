extern void input();
extern void output();
extern void space();
extern void nextline();


int a[100000000];
int pa1,pa2,res;
int n,data;

void partition(){
	int p,r,x,i,j,t;
	p=pa1;r=pa2;
	x=a[r];
	i=p-1;
	j=p;
	while (j<r) {
		if (a[j]<=x) {
			i=i+1;
			t=a[i];
			a[i]=a[j];
			a[j]=t;
		}
		j=j+1;
	}
	t=a[r];
	a[r]=a[i+1];
	a[i+1]=t;
	res=i+1;
}

void quicksort(){
	int p,r,q; 
	p=pa1;r=pa2;
	if (p<r) {
		pa1=p;pa2=r;partition();q=res;
		pa1=p;pa2=q-1;quicksort();
		pa1=q+1;pa2=r;quicksort();
	}	
}

void main(){
	int i;
	input();n=data;
	i=0;
	while (i<n) {
		input();
		a[i]=data;
		i=i+1;
	}
	pa1=0;pa2=n-1;
	quicksort();
	i=0;
	while (i<n) {
		data=a[i];
		output();
		space();
		i=i+1;
	}
	nextline();
}
