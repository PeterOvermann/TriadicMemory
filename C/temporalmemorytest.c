#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "triadicmemory.h"
#include "encoders.h"

typedef struct
{
	TriadicMemory* M1, * M2;
	SDR* x, * y, * c, * u, * v, * prediction;
} TemporalMemory;

TemporalMemory* temporalmemory_new(int n, int p);	// constructor
SDR* temporalmemory(TemporalMemory*, SDR*);			// predictor

TemporalMemory* temporalmemory_new(int n, int p)
{
	TemporalMemory* T = (TemporalMemory * )malloc(sizeof(TemporalMemory));

	T->M1 = triadicmemory_new(n, p);
	T->M2 = triadicmemory_new(n, p);

	T->x = sdr_new(n);	// persistent circuit state variables
	T->y = sdr_new(n);
	T->c = sdr_new(n);
	T->u = sdr_new(n);
	T->v = sdr_new(n);
	T->prediction = sdr_new(n);

	return T;
}

SDR* temporalmemory(TemporalMemory* T, SDR* inp)
{
	// flush state variables if input is zero -- needed for usage as sequence memory
	if (inp->p == 0)
	{
		T->y->p = T->c->p = T->u->p = T->v->p = T->prediction->p = 0;
		return T->prediction;
	}

	sdr_or(T->x, T->y, T->c);
	sdr_set(T->y, inp);

	if (!sdr_equal(T->prediction, T->y))
		// less aggressive test: if ( sdr_overlap (T->prediction, T->y) < T->M2->p)
		triadicmemory_write(T->M2, T->u, T->v, T->y);

	triadicmemory_read_z(T->M1, T->x, T->y, T->c); // recall c
	triadicmemory_read_x(T->M1, T->u, T->y, T->c); // recall u

	if (sdr_overlap(T->x, T->u) < T->M1->px)
	{
		sdr_random(T->c, T->M1->pz);
		triadicmemory_write(T->M1, T->x, T->y, T->c);
	}

	return triadicmemory_read_z(T->M2, sdr_set(T->u, T->x), sdr_set(T->v, T->y), T->prediction);
	// important: the return value is used in the next iteration and should not be changed by
	// the embedding function
}

void main(int argc, char* argv[])
{
	int N, P, Min = 0, Max = 120, man, count = 0, iter = 0;

	sscanf(argv[1], "%i", &N);
	sscanf(argv[2], "%i", &P);
	sscanf(argv[3], "%i", &man);	// 1: waits for ENTER for next prediction
									// 0: predicts continuously

	char str[7] = { 'b', 'a', 'n', 'a', 'n', 'a' };

	SDR* inp = sdr_new(N);

	TemporalMemory* T = temporalmemory_new(N, P);

	while (1) {
		if (count == 0) {
			printf("Iteration: %i\n", iter++);
		}

		int in = (int)str[count];

		count = ++count % strlen(str);

		printf("input: %i -> ", in);

		Int2SDR(inp, in, N, P, Min, Max);

		int pred = SDR2Int(temporalmemory(T, inp), N, P, Min, Max);

		printf("prediction: %i ", pred);

		if (iter == 1) printf("./.");
		else if (pred == str[count]) printf("correct");
		else printf("not correct");

		if (man == 1) getc(stdin);
		else printf("\n");
	}
}