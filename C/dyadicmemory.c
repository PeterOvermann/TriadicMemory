/*

dyadicmemory.c

Copyright (c) 2021-2022 Peter Overmann

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


This is the reference implementation of the hetero-associative memory published in the 2022 paper
https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf

The algorithm realizes a Sparse Distributed Memory (SDM) as envisioned by Pentti Kanerva (1988),
using an artificial neural network with combinatorial connectivity.

This stand-alone source file produces a command line tool. A library version of the same code
is available in triadicmemory.c.

Build the command line tool: cc -Ofast dyadicmemory.c -o /usr/local/bin/dyadicmemory

This command line tool instantiates a new memory instance.

It stores heteroassociations x->y of Sparse Distributed Representations (SDR) x and y,
and recalls y for a given x.

An SDR is given by a set of p integers in the range from 1 to n, representing the non-zero positions of the SDR.
(Note that the internal representation uses integers from 0 to n-1.)

Typical values are n = 1000 and p = 10

Command line usage: dyadicmemory 1000 10


Usage:

Store x->y:
1 20 195 355 371 471 603  814 911 999, 13 29 41 182 590 711 714 773 925 967

Recall y:
1 20 195 355 371 471 603  814 911 999

Delete x->y:
- 1 20 195 355 371 471 603  814 911 999, 13 29 41 182 590 711 714 773 925 967

Terminate process:
quit

Print version number:
version

*/


#define VERSIONMAJOR 1
#define VERSIONMINOR 2

#define SEPARATOR ','

#define MEMORYTYPE unsigned char

#define INPUTBUFFER 10000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


#define TMEMTYPE unsigned char


// various SDR utilities:

typedef struct
	{
	int *a, n, p;
	} SDR;
	
	
SDR *sdr_new(int n)
	{
	SDR *s = malloc(sizeof(SDR));
	s->a = malloc(n*sizeof(int));
	s->n = n;
	s->p = 0;
	return s;
	}
	
void sdr_print(SDR *s)
	{
	for (int r = 0; r < s->p; r++)
		{
		printf("%d", s->a[r] + 1);  // adding 1 because input/output values range from 1 to N
		if (r < s->p -1) printf(" ");
		}
	printf("\n"); fflush(stdout);
	}
	

// dyadic (hetero-associative) memory

typedef struct
	{
	TMEMTYPE* m;
	int n, p;
	} DyadicMemory;


DyadicMemory *dyadicmemory_new(int n, int p)
	{
	DyadicMemory *a = malloc(sizeof(DyadicMemory));
	
	// limitation: malloc may fail for large n, use virtual memory instead in this case
	a->m = (TMEMTYPE*) malloc( n*n*(n-1)/2 * sizeof(TMEMTYPE));
	
	a->n = n;
	a->p = p;

	for(int i = 0; i < n*n*(n-1)/2 ; i++)  *(a->m + i) = 0; // memory initialization
	
	return a;
	}
	
void dyadicmemory_write (DyadicMemory *D, SDR *x, SDR *y)
	{
	int N = D->n;
	
	for( int i = 0; i < x->p - 1; i++)
		for( int j = i+1; j < x->p; j++)
			{
			int u = N *(-2 - 3*x->a[i] - x->a[i]*x->a[i] + 2*x->a[j] + 2*x->a[i]*N) / 2 ;
				for( int k = 0; k < y->p; k++)
					++ *( D->m + u + y->a[k] );
			}
	}
	
	
void dyadicmemory_delete (DyadicMemory *D, SDR *x, SDR *y)
	{
	int N = D->n;
	
	for( int i = 0; i < x->p - 1; i++)
		for( int j = i+1; j < x->p; j++)
			{
			int u = N *(-2 - 3*x->a[i] - x->a[i]*x->a[i] + 2*x->a[j] + 2*x->a[i]*N) / 2 ;
				for( int k = 0; k < y->p; k++)
					if (*( D->m + u + y->a[k] ) > 0) // test for counter underflow
						-- *( D->m + u + y->a[k] );
			}
	}
	
	
static int cmpfunc (const void * a, const void * b)
	{ return  *(int*)a - *(int*)b; }
	
	
static SDR* binarize (SDR *v, int targetsparsity)
	{
	int sorted[v->n], rankedmax;
	
	for ( int i=0; i < v->n; i++ )
		sorted[i] = v->a[i];
	
	qsort( sorted, v->n, sizeof(int), cmpfunc);
	
	rankedmax = sorted[ v->n - targetsparsity ];
	
	if(rankedmax == 0)
		rankedmax = 1;
		
	v->p = 0;
	for ( int i = 0; i < v->n; i++)
		if (v->a[i] >= rankedmax)
			v->a[v->p++] = i;
	
	return (v);
	}
	
	
SDR* dyadicmemory_read (DyadicMemory *D, SDR *x, SDR *y)
	{
	int N = D->n;
	
	for( int k = 0; k < N; k++ ) y->a[k] = 0;
			
	for( int i = 0; i < x->p-1; i++)
		for( int j = i + 1; j < x->p; j++)
			{
			int u = N *(-2 - 3*x->a[i] - x->a[i]*x->a[i] + 2*x->a[j] + 2*x->a[i]*N) / 2 ;
			for( int k = 0; k < N; k++)
				y->a[k] += *( D->m + u + k);
			}
						
	return( binarize(y, D->p));
	}
	
	

static char* parse (char *buf, SDR *s)
	{
	int *i;
	s->p = 0;
	
	while ( *buf  && *buf != SEPARATOR )
		{
		while (isspace(*buf)) buf++;
		if (! isdigit(*buf)) break;
		
		i = s->a + s->p;
		sscanf( buf, "%d", i);
		
		if ( (*i)-- > s->n || *i < 0 ) // subtracting 1 because internal representation range is 0 to N-1
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
	char *buf, inputline[INPUTBUFFER];
	
	int N, P;  // vector dimension and target sparse population
	
	if (argc != 3)
		{
		printf("usage: dyadicmemory n p\n");
		printf("n is the hypervector dimension, typically 1000\n");
		printf("p is the target sparse population, typically 10\n");
		exit(1);
		}
        
    sscanf( argv[1], "%d", &N);
    sscanf( argv[2], "%d", &P);
    
    SDR *x = sdr_new(N);
	SDR *y = sdr_new(N);
	
	DyadicMemory *D = dyadicmemory_new(N, P);

	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{
		if ( strcmp(inputline, "quit\n") == 0)
			return 0;
			
		else if ( strcmp(inputline, "version\n") == 0)
			printf("%d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
		else // parse x
			{
			int delete = 0;
			
			if (*inputline == '-')
				delete = 1;
			
			buf = parse(inputline + delete, x);
			
			if (*buf == SEPARATOR) // parse y
				{
				parse(buf+1, y);
				
				// store or delete x->y
				if (delete)
					dyadicmemory_delete (D, x,y);
				else
					dyadicmemory_write (D, x,y);
				}
				
			else if (*buf == 0) // query
				{
				dyadicmemory_read (D, x, y);
				sdr_print(y);
				}
			else
				{
				printf("invalid input\n");
				exit(5);
				}
			}
		}
	
	return 0;
	}
