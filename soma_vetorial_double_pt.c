#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#define __USE_GNU
#include <sched.h>
#undef __USE_GNU
#include <errno.h>
#include <math.h>
#include <pthread.h>

double waste_time(long n);
void *soma_vet(void *args);

struct call_args
{
	double *a, *b, *c;
	int size, t;
};

int main(int argc, char **argv)
{
	int status;
	pthread_t tid[2];
	pthread_attr_t attr[2];
	struct call_args my_args[2];
	FILE *inp_a, *inp_b, *out_c;
	long int size = atol(argv[4]);
	long int i;
	double *a, *b, *c;
	a = (double *) malloc(sizeof(double) * size);
	b = (double *) malloc(sizeof(double) * size);
	c = (double *) malloc(sizeof(double) * size);
	inp_a = fopen(argv[1], "r");
	inp_b = fopen(argv[2], "r");
	for (i=0; i<size; i++) 
	{
		fscanf(inp_a, "%lf\n", &a[i]);
		fscanf(inp_b, "%lf\n", &b[i]);
	}
	fclose(inp_a);
	fclose(inp_b);

	for(i=0; i<2; i++)
	{
		my_args[i].a=a;
		my_args[i].b=b;
		my_args[i].c=c;
		my_args[i].size=size;
		my_args[i].t=i;
	
		pthread_attr_init(&attr[i]);
		pthread_create(&tid[i], &attr[i],soma_vet, &my_args[i]);
	}
	for(i=0; i<2; i++)
	{
		pthread_join(tid[i],NULL);
	}
	out_c = fopen(argv[3], "w");
	for (i=0; i<size; i++) 
	{
		fprintf(out_c, "%lf\n", c[i]);
	}
	fclose(out_c);
	return 0;
}

double waste_time(long n)
{
    double res = 0;
    long i = 0;
    while(i <n * 200) {
        i++;
        res += sqrt (i);
    }
    return res;
}

void *soma_vet(void *args)
{
	struct call_args *m = (struct call_args *) args;
	int i;
	for (i=m->size/2*m->t; i<m->size/2*(m->t+1);i++)
		m->c[i]=waste_time(abs(m->a[i]))+waste_time(abs(m->b[i]));
}

