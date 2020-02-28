/*	VLAD ANDREI-ALEXANDRU	*/
/*			331CB			*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define ERROR_PARAMETERS 	-1
#define ERROR_FILES 		-2
#define ERROR_MALLOC		-3

#define PI 3.14159265358979323846

typedef struct{
	double re;
	double im;
} Complex;

int N;
int P;
double* xi;
Complex* xk;

void* fourrierTransform(void *args) {
	int k;
	int t;

	// Calculez start si end pentru fiecare thread (formule ca la laborator)
	int thread_id = *(int*)args;
	int start = thread_id * ceil((double)N/P);
	int end =  fmin((thread_id + 1) * ceil((double)N/P), N);
	
	// Fiecare thread calculeaza cat ii specific folosind formula
	for (k = start; k < end; k++) {
		double sumRe = 0;
		double sumIm = 0;

		for (t = 0; t < N; t++) {
			double angle = 2 * PI * t * k / N;
			sumRe += xi[t] * cos(angle);
			sumIm += -xi[t] * sin(angle);
		}

		xk[k].re = sumRe;
		xk[k].im = sumIm;
	}

	return NULL;
}

void usage() {
	printf("Usage: ./homeworkFT inputValues.txt outputValues.txt numThreads\n");
	exit(ERROR_PARAMETERS);
}

int main(int argc, char * argv[]) {
	int i;
	int dump;

	// Verifica argumentele
	if(argc != 4) { usage(); }

	// Deschide fisierele
	FILE* input		= (FILE*) fopen(argv[1], "r");
	FILE* output	= (FILE*) fopen(argv[2], "wo");

	// Verifica daca s-au deschis cu succes
	if(!input || !output) {
		printf("File(s) could not be open\n");
		exit(ERROR_FILES);
	}

	// Citeste numarul de threaduri
	int threads = atoi(argv[3]);
	P = threads;
	printf("Number of threads: %d\n", threads);

	// Citeste N
	dump = fscanf(input, "%d", &N);
	if(dump) {
		fprintf(output, "%d\n", N);
	}

	// Aloc vectori pentru datele initiale si datele rezultat
	xi = (double*) malloc(sizeof(double) * N);		// input
	xk = (Complex*) malloc(sizeof(Complex) * N);	// output
	if(!xi || !xk) {
		printf("Malloc failed\n");
		exit(ERROR_MALLOC);
	}
	
	// Citeste cele N numere
	for(i = 0; i < N; i++) {
		dump = fscanf(input, "%lf", &xi[i]);
	}

	// Initializeaza threadurile
	pthread_t tid[threads];
	int thread_id[threads];
	for(i = 0;i < threads; i++)
		thread_id[i] = i;

	for(i = 0; i < threads; i++) {
		pthread_create(&(tid[i]), NULL, fourrierTransform, &(thread_id[i]));
	}

	for(i = 0; i < threads; i++) {
		pthread_join(tid[i], NULL);
	}

	// Scrie vectorul returnat in fisierul de output
	for(i = 0; i < N; i++) {
		fprintf(output, "%f %f\n", xk[i].re, xk[i].im);
	}

	fclose(input);
	fclose(output);

	free(xi);
	free(xk);

	return 0;
}


