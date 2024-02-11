/*
deeptemporalmemory.c
Deep Temporal Memory Implementation Template and Command Line Tool Wrapper
https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf

Copyright (c) 2022-2024 Peter Overmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the “Software”), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


/*

This is an implentation template for a Deep Temporal Memory.
 

Deep Temporal Memory processes a temporal stream of sparse distributed representations (SDRs),
at each step predicting the next step, based on items seen before.

This algorithm can be used to predict the next step in a continuous stream of items, or to
learn individual sequences terminated by a zero-value SDR.

This command line tool instantiates a new instance of a Temporal Memory.

An SDR is given by a set of p integers in the range from 1 to n.
Typical values are n = 1000 and p = 5.

Command line arguments: temporalmemory <n> <p>

Command line usage:
256 381 438 479 904

Print random SDR:
random

Print version number:
version

Terminate process:
quit

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "triadicmemory.h"



// ---------- Temporal Bigram Encoder  ----------
		
		
typedef struct
	{
	TriadicMemory *T;
	SDR *x, *y, *z, *u;
	} TemporalBigramEncoder;
	
TemporalBigramEncoder* temporalbigramencoder_new (int n, int p);	// constructor
SDR* temporalbigramencoder (TemporalBigramEncoder *, SDR *);		// predictor



TemporalBigramEncoder* temporalbigramencoder_new (int n, int p)
	{
	TemporalBigramEncoder *R = malloc( sizeof(TemporalBigramEncoder));
	
	R->T = triadicmemory_new(n, p);
	
	R->x = sdr_new(n);	// persistent circuit state variables
	R->y = sdr_new(n);
	R->z = sdr_new(n);
	
	R->u = sdr_new(n);	// temporary variable
	return R;
	}

SDR* temporalbigramencoder (TemporalBigramEncoder *R, SDR *inp)
	{
	// flush state variables if input is zero -- needed for usage as sequence memory
	if (inp->p == 0)
		{
		R->x->p = R->y->p = R->z->p = 0;
		return R->z;
		}
	
	sdr_or (R->x, R->y, R->z);
	sdr_set(R->y, inp);
	
	if (R->x->p == 0)
		return R->z;
	
	triadicmemory_read_z (R->T, R->x, R->y, R->z); // recall z
	triadicmemory_read_x (R->T, R->u, R->y, R->z); // recall u (temp variable)
	
	if (sdr_overlap(R->x, R->u) < R->T->px)
		{
		sdr_random( R->z, R->T->pz);
		triadicmemory_write( R->T, R->x, R->y, R->z);
		}
		
	return R->z;
	}

		
		
// ---------- Deep Temporal Memory  ----------

typedef struct
	{
	TriadicMemory *M;
	SDR *x, *y, *z;
	
	TemporalBigramEncoder *R1, *R2, *R3, *R4, *R5, *R6, *R7;
	SDR *t1, *t2, *t3, *t4, *t5, *t6, *t7;
	
	} DeepTemporalMemory;
	
DeepTemporalMemory* deeptemporalmemory_new (int n, int p);	// constructor
SDR* deeptemporalmemory (DeepTemporalMemory *, SDR *);		// predictor



DeepTemporalMemory* deeptemporalmemory_new (int n, int p)
	{
	DeepTemporalMemory *D = malloc( sizeof(DeepTemporalMemory));
	
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
	
	
SDR* deeptemporalmemory (DeepTemporalMemory *D, SDR *inp)
	{
	// if inp is zero, all state variables will go to zero in this function pass
	
	
	// prediction not correct? store new prediction
	
	if ( ! sdr_equal (D->z, inp) )
		triadicmemory_write( D->M, D->x, D->y, inp );
		
	// TemporalBigramEncoding chain
	
	D->t1 = temporalbigramencoder (D->R1, inp);	// t1 is a temporal 2gram
	D->t2 = temporalbigramencoder (D->R2, D->t1);
	D->t3 = temporalbigramencoder (D->R3, D->t2);
	D->t4 = temporalbigramencoder (D->R4, D->t3);
	D->t5 = temporalbigramencoder (D->R5, D->t4);
	D->t6 = temporalbigramencoder (D->R6, D->t5);
	D->t7 = temporalbigramencoder (D->R7, D->t6); 	// t7 is a temporal 8gram
	
	// prediction based on readout from t1, t2, t4 and t7 (other combinations work as well)

	sdr_or (D->x, D->t1, D->t4);
	sdr_or (D->y, D->t2, D->t7);

	return triadicmemory_read_z (D->M, D->x, D->y, D->z);
	// important: the return value is used in the next iteration and should not be changed by
	// the embedding function
	}

		
// ---------- Command Line Tool   ----------
	
		
static int VERSIONMAJOR = 1;
static int VERSIONMINOR = 0;


int main(int argc, char *argv[])
	{
	char inputline[10000];
	
	if (argc != 3)
		{
		printf("usage: deeptemporalmemory <n> <p>\n");
		printf("n is the hypervector dimension    (typical value 1000)\n");
		printf("p is the target sparse population (typical value 5)\n");
		exit(1);
		}
        
	int N, P;  // SDR dimension and target sparse population, received from command line

	sscanf( argv[1], "%d", &N);
	sscanf( argv[2], "%d", &P);
    
    	DeepTemporalMemory *T = deeptemporalmemory_new (N, P);
   
	SDR *inp = sdr_new(N);
	SDR *out = sdr_new(N);
	
	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{
		if (! strcmp(inputline, "quit\n"))
			exit(0);

		if (! strcmp(inputline, "random\n"))
			sdr_print(sdr_random(out, P));

		else if ( strcmp(inputline, "version\n") == 0)
			printf("deeptemporalmemory %d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
		else // parse input SDR
			{
			if (* sdr_parse(inputline, inp) )
				{
				printf("unexpected input: %s", inputline);
				exit(5);
				}
			sdr_print( deeptemporalmemory(T, inp));
			}
		}
			
	return 0;
	}
	
