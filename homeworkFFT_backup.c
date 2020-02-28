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

typedef double real;
typedef struct{real Re; real Im;} complex;

typedef struct{
	complex* v;
	complex* tmp;
	int end;
	int level;
} Args;

int P;


// void* fft(void* a)
// {
// 	Args* args = (Args*) a;

//   if((args->end)>1) {			/* otherwise, do nothing and return */
//     int k,m;
// 	complex z, w, *vo, *ve;

//     ve = args->tmp; vo = args->tmp+(args->end)/2;
//     for(k=0; k<(args->end)/2; k++) {
//       ve[k] = args->v[2*k];
//       vo[k] = args->v[2*k+1];
//     }

//     fft( ve, (args->end)/2, args->v );		/* FFT on even-indexed elements of args->v[] */
//     fft( vo, (args->end)/2, args->v );		/* FFT on odd-indexed elements of args->v[] */
    
// 	for(m=0; m<(args->end)/2; m++) {
//       w.Re = cos(2*PI*m/(double)(args->end));
//       w.Im = -sin(2*PI*m/(double)(args->end));
//       z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
//       z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
//       args->v[  m  ].Re = ve[m].Re + z.Re;
//       args->v[  m  ].Im = ve[m].Im + z.Im;
//       args->v[m+(args->end)/2].Re = ve[m].Re - z.Re;
//       args->v[m+(args->end)/2].Im = ve[m].Im - z.Im;
//     }
//   }
//   return;
// }



void* fft(void* a)
{
	Args* args = (Args*) a;

  if(args->end > 1) {			/* otherwise, do nothing and return */
    int k,m;
	complex z, w, *vo, *ve;

    ve = args->tmp; vo = args->tmp+(args->end)/2;
    for(k=0; k<(args->end)/2; k++) {
      ve[k] = args->v[2*k];
      vo[k] = args->v[2*k+1];
    }


	// pthread_barrier_t barrier;

	if(P != 1 && pow(2, args->level) <= P) {

		// printf("FACE THREAD-URI la nivel %d\n", args->level);

		// pthread_barrier_init(&barrier, NULL, 2);
		int threads = 2;
		pthread_t tid[threads];

		// FFT on even-indexed elements
		Args argEven;
		argEven.v = ve;
		argEven.tmp = args->v;
		argEven.end = (args->end)/2;
		argEven.level = args->level + 1;
		pthread_create(&(tid[0]), NULL, fft, &argEven);

		// FFT on odd-indexed elements
		Args argOdd;
		argOdd.v = vo;
		argOdd.tmp = args->v;
		argOdd.end = (args->end)/2;
		argOdd.level = args->level + 1;
		pthread_create(&(tid[1]), NULL, fft, &argOdd);

		pthread_join(tid[0], NULL);
		pthread_join(tid[1], NULL);

		// WAIT TO FINISH
		// pthread_barrier_wait(&barrier);
		// printf("A trecut de bariera la nivelul %d\n", args->level);

		// Destroy barrier
		// pthread_barrier_destroy(&barrier);
	}else {
		printf("UN SINGUR THREAD LA LEVEL=%d\n", args->level);


		Args argEven;
		argEven.v = ve;
		argEven.tmp = args->v;
		argEven.end = (args->end)/2;
		argEven.level = args->level + 1;

		Args argOdd;
		argOdd.v = vo;
		argOdd.tmp = args->v;
		argOdd.end = (args->end)/2;
		argOdd.level = args->level + 1;
	    fft(&argEven);		/* FFT on even-indexed elements of v[] */
    	fft(&argOdd);		/* FFT on odd-indexed elements of v[] */
	}

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);

	for(m=0; m<(args->end)/2; m++) {
      w.Re = cos(2*PI*m/(double)(args->end));
      w.Im = -sin(2*PI*m/(double)(args->end));
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      args->v[  m  ].Re = ve[m].Re + z.Re;
      args->v[  m  ].Im = ve[m].Im + z.Im;
      args->v[m+(args->end)/2].Re = ve[m].Re - z.Re;
      args->v[m+(args->end)/2].Im = ve[m].Im - z.Im;
    }

	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

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
	complex* v;
	complex* tmp;

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

	v = (complex*)malloc(sizeof(complex) * N);
	tmp = (complex*)malloc(sizeof(complex) * N);
	if(!v || !tmp) {
		printf("Malloc failed\n");
		fclose(input);
		fclose(output);
		exit(ERROR_MALLOC);
	}

	// Citeste cele N numere
	for(i = 0; i < N; i++) {
		dump = fscanf(input, "%lf", &v[i].Re);
		v[i].Im = 0.0f;
	}


	// // Initializeaza threadurile
	// pthread_t tid[threads];
	// int thread_id[threads];
	// for(i = 0;i < threads; i++)
	// 	thread_id[i] = i;

	// for(i = 0; i < threads; i++) {
	// 	pthread_create(&(tid[i]), NULL, fft, &(thread_id[i]));
	// }

	// for(i = 0; i < threads; i++) {
	// 	pthread_join(tid[i], NULL);
	// }
	
	Args arg;
	arg.v = v;
	arg.tmp = tmp;
	arg.end = N;
	arg.level = 0;
	fft(&arg);

	// Scrie vectorul returnat in fisierul de output
	for(i = 0; i < N; i++) {
		fprintf(output, "%f %f\n", v[i].Re, v[i].Im);
	}

	fclose(input);
	fclose(output);

	free(v);
	free(tmp);

	return 0;
}
