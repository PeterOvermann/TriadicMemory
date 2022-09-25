#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "triadicmemory.h"
#include "encoders.h"

typedef struct
{
	TriadicMemory* T;
	SDR* x, * y, * z, * u;
} TemporalBigramEncoder;

TemporalBigramEncoder* temporalbigramencoder_new(int n, int p);	// constructor
SDR* temporalbigramencoder(TemporalBigramEncoder*, SDR*);		// predictor


TemporalBigramEncoder* temporalbigramencoder_new(int n, int p)
{
	TemporalBigramEncoder* R = malloc(sizeof(TemporalBigramEncoder));

	R->T = triadicmemory_new(n, p);

	R->x = sdr_new(n);	// persistent circuit state variables
	R->y = sdr_new(n);
	R->z = sdr_new(n);

	R->u = sdr_new(n);	// temporary variable
	return R;
}


SDR* temporalbigramencoder(TemporalBigramEncoder* R, SDR* inp)
{
	// flush state variables if input is zero -- needed for usage as sequence memory
	if (inp->p == 0)
	{
		R->x->p = R->y->p = R->z->p = 0;
		return R->z;
	}

	sdr_or(R->x, R->y, R->z);
	sdr_set(R->y, inp);

	if (R->x->p == 0)
		return R->z;

	triadicmemory_read_z(R->T, R->x, R->y, R->z); // recall z
	triadicmemory_read_x(R->T, R->u, R->y, R->z); // recall u (temp variable)

	if (sdr_overlap(R->x, R->u) < R->T->px)
	{
		sdr_random(R->z, R->T->pz);
		triadicmemory_write(R->T, R->x, R->y, R->z);
	}

	return R->z;
}


typedef struct
{
	TriadicMemory* M;
	SDR* x, * y, * z;

	TemporalBigramEncoder* R1, * R2, * R3, * R4, * R5, * R6, * R7;
	SDR* t1, * t2, * t3, * t4, * t5, * t6, * t7;

} DeepTemporalMemory;


DeepTemporalMemory* deeptemporalmemory_new(int n, int p);	// constructor
SDR* deeptemporalmemory(DeepTemporalMemory*, SDR*);		// predictor


DeepTemporalMemory* deeptemporalmemory_new(int n, int p)
{
	DeepTemporalMemory* D = malloc(sizeof(DeepTemporalMemory));

	D->M = triadicmemory_new(n, p);

	D->x = sdr_new(n);
	D->y = sdr_new(n);
	D->z = sdr_new(n);

	D->R1 = temporalbigramencoder_new(n, p);
	D->R2 = temporalbigramencoder_new(n, p);
	D->R3 = temporalbigramencoder_new(n, p);
	D->R4 = temporalbigramencoder_new(n, p);
	D->R5 = temporalbigramencoder_new(n, p);
	D->R6 = temporalbigramencoder_new(n, p);
	D->R7 = temporalbigramencoder_new(n, p);

	D->t1 = sdr_new(n);
	D->t2 = sdr_new(n);
	D->t3 = sdr_new(n);
	D->t4 = sdr_new(n);
	D->t5 = sdr_new(n);
	D->t6 = sdr_new(n);
	D->t7 = sdr_new(n);

	return D;
}


SDR* deeptemporalmemory(DeepTemporalMemory* D, SDR* inp)
{
	// if inp is zero, all state variables will go to zero in this function pass


	// prediction not correct? store new prediction

	if (!sdr_equal(D->z, inp))
		triadicmemory_write(D->M, D->x, D->y, inp);

	// TemporalBigramEncoding chain

	D->t1 = temporalbigramencoder(D->R1, inp);	// t1 is a temporal 2gram
	D->t2 = temporalbigramencoder(D->R2, D->t1);
	D->t3 = temporalbigramencoder(D->R3, D->t2);
	D->t4 = temporalbigramencoder(D->R4, D->t3);
	D->t5 = temporalbigramencoder(D->R5, D->t4);
	D->t6 = temporalbigramencoder(D->R6, D->t5);
	D->t7 = temporalbigramencoder(D->R7, D->t6); 	// t7 is a temporal 8gram

	// prediction based on readout from t1, t2, t4 and t7 (other combinations work as well)

	sdr_or(D->x, D->t1, D->t4);
	sdr_or(D->y, D->t2, D->t7);

	return triadicmemory_read_z(D->M, D->x, D->y, D->z);
	// important: the return value is used in the next iteration and should not be changed by
	// the embedding function
}

void main(int argc, char* argv[])
{
	/*
	// Prepare data set: remove delimiters -1 and -2 from original data set
	FILE* pIn = fopen("SIGN.txt", "r");

	if (!pIn) {
		printf("Cannot open file SIGN.txt\n\n");
		return;
	}

	FILE* pOut = fopen("SIGN_processed.txt", "w");

	int val;

	while (fscanf(pIn, "%i", &val) == 1) {
		if (val != -1 && val != -2) {
			//printf("%i ", val);
			fprintf(pOut, "%i ", val);
		}

		// next sequence
		if (val == -2) {
			fprintf(pOut, "\n", val);
		}
	}

	fclose(pIn);
	fclose(pOut);
	*/

	int N, P, Min = 0, Max = 500, iter = 0;

	sscanf(argv[1], "%i", &N);
	sscanf(argv[2], "%i", &P);

	FILE* pIn = fopen("SIGN_processed.txt", "r");

	if (!pIn) {
		printf("Can't open SIGN_processed.txt\n\n");
		return;
	}

	FILE* pOut = fopen("results.txt", "w");

	char* sequence = (char*)malloc(500 * sizeof(char));
	int val, val_count = 0, correct = 0, total = 0, pred_old = 0, sequ = 0;
	SDR* inp = sdr_new(N);
	DeepTemporalMemory* T = deeptemporalmemory_new(N, P);

	while (iter++ < 8) {

		total = 0;		// counts predicted values per iteration
		correct = 0;	// counts correct predictions per iteration
		sequ = 0;		// counts sequences per iteration

		// process sequences one by one
		while (fgets(sequence, 500, pIn) != NULL) {

			printf("iteration %i sequence %i\n", iter, ++sequ);

			val_count = 0;	// counts values per sequence

			// save sequence start address for later recovery
			char* tmp = sequence;
				
			// extract and process values from sequence one by one
			while (sscanf(sequence, "%i", &val) == 1) {

				//printf("%i ", val);

				if (val < Min || val > Max) {
					printf("Value out of range\n\n");
					return;
				}
				
				val_count++;

				// value -> SDR
				Int2SDR(inp, val, N, P, Min, Max);

				// predict next value
				int pred = SDR2Int(deeptemporalmemory(T, inp), N, P, Min, Max);

				if (pred_old == val) correct++;
				if (val_count > 3) total++;		// don't count first three values in sequence

				pred_old = pred;

				// move pointer to next value (the sequence start address is lost here,
				// but has been saved above to recover it for the next sequence)
				while (*(++sequence) != ' ');	
			}

			// recover sequence start address for the next sequence
			sequence = tmp;

			// flush memory state
			deeptemporalmemory(T, sdr_new(N));
		}

		printf("\niteration %i\n", iter);
		printf("%f percent correct\n\n", 100.0 * (float)correct / (float)total);

		fprintf(pOut, "iteration %i\n", iter);
		fprintf(pOut, "%f percent correct\n\n", 100.0 * (float)correct / (float)total);

		rewind(pIn);
	}

	free(sequence);

	fclose(pIn);
	fclose(pOut);
}