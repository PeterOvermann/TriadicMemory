/*
temporalmemory.c

Copyright (c) 2022 Peter Overmann

C-language implementation of the temporal memory algorithm published in
   https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf
This source can be compiled as a stand-alone command line program or as a library.

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

Building the command line version:
add in triadicmemory.h: #define TRIADICMEMORY_LIBRARY
cc -Ofast temporalmemory.c triadicmemory.c -o /usr/local/bin/temporalmemory

An SDR is given by a sorted list of p integers in the range from 1 to n.
Typical values are n = 1000 and p = 10 to 20.

Command line arguments: temporalmemory <n> <p>

Command line usage examples:

Input an SDR (the response/prediction will be of the same form, or an empty line)
99 175 180 222 337 408 416 417 430 525 584 597 617 637 643 685 733 876 917 980

Print a random SDR:
random

Print version number:
version

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "triadicmemory.h"

// declarations

typedef struct
	{
	TriadicMemory *M1, *M2;
	SDR *x, *y, *c, *u, *v, *prediction;
	} TemporalMemory;
	
TemporalMemory* temporalmemory_new (int n, int p);						// constructor
SDR* temporalmemory_predict (TemporalMemory *T, SDR *inp);				// predictor

// end of declarations


// uncomment the following line if compiling as library
// #define TEMPORALMEMORY_LIBRARY

TemporalMemory* temporalmemory_new (int n, int p)
	{
	TemporalMemory *T = malloc( sizeof(TemporalMemory));
	
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
	
	
SDR* temporalmemory_predict (TemporalMemory *T, SDR *inp)
	{
	sdr_or (T->x, T->y, T->c);
	sdr_set(T->y, inp);
	
	if ( ! sdr_equal (T->prediction, T->y) )
	// less aggressive test: if ( sdr_overlap (T->prediction, T->y) < T->M2->p)
		triadicmemory_write( T->M2, T->u, T->v, T->y );
		
	triadicmemory_read_z (T->M1, T->x, T->y, T->c); // recall c
	triadicmemory_read_x (T->M1, T->u, T->y, T->c); // recall u
	
	if (sdr_overlap(T->x, T->u) < T->M1->p)
		{
		sdr_random( T->c, T->M1->p);
		triadicmemory_write( T->M1, T->x, T->y, T->c);
		}
		
	return (triadicmemory_read_z (T->M2, sdr_set(T->u, T->x), sdr_set(T->v, T->y), T->prediction) );
	// important: the return value is used in the next iteration and should not be changed by
	// the embedding function
	}
	
#ifndef TEMPORALMEMORY_LIBRARY
		
#define VERSIONMAJOR 1
#define VERSIONMINOR 1

static char* parse (char *buf, SDR *s)
	{
	int *i;
	s->p = 0;
	
	while ( *buf != 0 )
		{
		while (isspace(*buf)) buf++;
		if (! isdigit(*buf)) break;
		
		i = s->a + s->p;
		sscanf( buf, "%d", i);
		
		if ( (*i)-- > s->n || *i < 0 )
			{
			printf("position out of range: %s\n", buf);
			exit(2);
			}
		s->p ++;
		
		while (isdigit(*buf)) buf++;
		while (isspace(*buf)) buf++;
		}
		
	return buf;
	}


int main(int argc, char *argv[])
	{
	char inputline[10000];
	
	if (argc != 3)
		{
		printf("usage: temporalmemory <n> <p>\n");
		printf("n is the hypervector dimension    (typical value 1000)\n");
		printf("p is the target sparse population (typical value 10 to 20)\n");
		exit(1);
		}
        
	int N, P;  // SDR dimension and target sparse population, received from command line

    sscanf( argv[1], "%d", &N);
    sscanf( argv[2], "%d", &P);
    
    TemporalMemory *T = temporalmemory_new (N, P);
   
	SDR *inp = sdr_new(N);
	SDR *out = sdr_new(N);
	
	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{
		if (! strcmp(inputline, "random\n"))
			sdr_print(sdr_random(out, P));

		else if ( strcmp(inputline, "version\n") == 0)
			printf("%d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
		else // parse input SDR
			{
			if (* parse(inputline, inp) )
				{
				printf("unexpected input: %s", inputline);
				exit(5);
				}
			sdr_print( temporalmemory_predict(T, inp));
			}
		}
			
	return 0;
	}
	
#endif // TEMPORALMEMORY_LIBRARY
