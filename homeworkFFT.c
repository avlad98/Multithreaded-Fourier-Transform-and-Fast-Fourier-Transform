/*	VLAD ANDREI-ALEXANDRU	*/
/*			331CB			*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <complex.h>
#include <string.h>

#define ERROR_PARAMETERS 	-1
#define ERROR_FILES 		-2
#define ERROR_MALLOC		-3

#define PI 3.14159265358979323846

int P;

typedef double complex cplx;

// Structura care contine toti parametrii de care am nevoie
typedef struct {
	cplx* v;		// vector cu date initiale
	cplx* out;		// vector cu rezultatul
	int n;			// limita superioara (pana unde se calculeaza pt fiecare thread)
	int step;		
	int level;		// nivelul pe care ma aflu in arbore (porneste de la 0)
} Args;


// Functia fft de pe rosetta paralelizata
void* _fft(void* a)
{
	Args* args = (Args*)a;

	// Conditia de oprire a recursivitatii
	if ((args->step) < args->n) {
		
		// Creez cele doua argumente pentru cele doua
		// apeluri recursive
		Args fst, snd;
		
		fst.v = args->out;
		fst.out = args->v;
		fst.n = args->n;
		fst.step = (args->step) * 2;
		fst.level = args->level + 1;

		snd.v = args->out + (args->step);
		snd.out = args->v + (args->step);
		snd.n = args->n;
		snd.step = (args->step) * 2;
		snd.level = args->level + 1;

		// Verific daca trebuie sa mai paralelizez de la nivelul curent
		if(P != 1 && args->level <= P) {
			/*
				Daca am mai multe threaduri verific pe ce nivel ma aflu si daca se mai merita
				sa paralelizez.
				Daca vreau sa pornesc functia pe 4 core-uri, primul apel de functie genereaza
				2 thread-uri si le lasa sa lucreze. La randul lor, fiecare din cele doua
				thread-uri verifica daca se mai merita paralelizat sau nu.	
			*/
			int i;
			int threads = 2;
			pthread_t tid[threads];

			pthread_create(&(tid[0]), NULL, _fft, &fst);
			pthread_create(&(tid[1]), NULL, _fft, &snd);

			for(i = 0; i < threads; i++) {
				pthread_join(tid[i], NULL);
			}
		}else {
			/* 
				Daca am un singur thread dat ca parametru atunci nu mai creez altul si 
				il las sa isi continue executia secventiala. 
			*/
			_fft(&fst);
			_fft(&snd);
		}

		// Aici deja s-au terminat cele doua thread-uri (daca au fost create) si
		// continua firul principal (care a nascut cele doua thread-uri) sa calculeze
		// mai departe
		for (int i = 0; i < args->n; i += 2 * (args->step)) {
			cplx t = cexp(-I * PI * i / args->n) * args->out[i + (args->step)];
			args->v[i / 2]     = args->out[i] + t;
			args->v[(i + args->n)/2] = args->out[i] - t;
		}
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
	int N;

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
	P = atoi(argv[3]);
	printf("Number of threads: %d\n", P);

	// Citeste N
	dump = fscanf(input, "%d", &N);
	if(dump) {
		fprintf(output, "%d\n", N);
	}

	cplx* v = (cplx*)malloc(sizeof(cplx) * N);
	cplx* out = (cplx*)malloc(sizeof(cplx) * N);

	// Citeste cele N numere
	for(i = 0; i < N; i++) {
		double temp;
		dump = fscanf(input, "%lf", &temp);
		v[i] = temp;
	}

	Args args;
		args.v = v;
		args.out = out;
		args.n = N;
		args.step = 1;
		args.level = 0;

	_fft(&args);

	// Scrie vectorul returnat in fisierul de output
	for(i = 0; i < N; i++) {
		fprintf(output, "%f %f\n", creal(v[i]), cimag(v[i]));
	}

	fclose(input);
	fclose(output);

	return 0;
}