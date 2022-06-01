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
using an artificial neural network with a combinatorial connectivity.

Build: cc -Ofast dyadicmemory.c -o /usr/local/bin/dyadicmemory

This command line tool instantiates a new memory instance.
It stores heteroassociations x->y of Sparse Distributed Representations (SDR) x and y,
and recalls y for a given x.

An SDR is given by a set of p integers in the range from 1 to n, representing the non-zero positions of the SDR.

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
#define VERSIONMINOR 1

#define SEPARATOR ','

#define MEMORYTYPE unsigned char

#define INPUTBUFFER 10000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


char* parse (char *buf, int *vec, int *len, int n)
	{
	int *p;
	*len = 0;
	
	while ( *buf != 0 && *buf != SEPARATOR )
		{
		while (isspace(*buf)) buf++;
		if (! isdigit(*buf)) break;
		
		p = vec + *len;
		sscanf( buf, "%d", p);
		
		if ( (*p)-- > n || *p < 0 )
			{
			printf("vector position out of range: %s\n", buf);
			exit(2);
			}
		(*len)++;
		
		while (isdigit(*buf)) buf++;
		while (isspace(*buf)) buf++;
		}
		
	
	return buf;
	}


int cmpfunc (const void * a, const void * b)
	{
	return  *(int*)a - *(int*)b;
	}
	
void binarize (int *v, int n, int p)
	{
	int i, sorted[n], rankedmax;
	
	for ( i=0; i < n; i++ )
		sorted[i] = v[i];
	
	qsort( sorted, n, sizeof(int), cmpfunc);
	
	rankedmax = sorted[ n - p ];
	
	if(rankedmax == 0)
		rankedmax = 1;
		
	for ( i=0; i<n; i++) if (v[i] >= rankedmax) printf( "%d ", i+1 );
	
	printf( "\n");
	fflush(stdout);
	}


int main(int argc, char *argv[])
	{
	char *buf, inputline[INPUTBUFFER];
	int *x, *y;
	int xmax, ymax;
	
	int delete;
	
	int i, j, k, u;
	int N, P;  // vector dimension and target sparse population
	
	MEMORYTYPE *T;
	
	if (argc != 3)
		{
		printf("usage: dyadicmemory n p\n");
		printf("n is the hypervector dimension, typically 1000\n");
		printf("p is the target sparse population, typically 10\n");
		exit(1);
		}
        
    sscanf( argv[1], "%d", &N);
    sscanf( argv[2], "%d", &P);
   
	x = (int*)malloc(N*sizeof(int));
	y = (int*)malloc(N*sizeof(int));
	
	// limitation: malloc may fail for large n, use virtual memory in this case
	T = (MEMORYTYPE*) malloc( N*N*(N-1)/2 * sizeof(MEMORYTYPE));

	for(i = 0; i < N*N*(N-1)/2 ; i++)  *(T + i) = 0; // memory initialization

	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{

		if ( strcmp(inputline, "quit\n") == 0)
			return 0;
			
		else if ( strcmp(inputline, "version\n") == 0)
			printf("%d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
		else // parse x
			{
			delete = 0;
			if (*inputline == '-')
				delete = 1;
			
			buf = parse(inputline + delete, x, &xmax, N);
			
			if (*buf == SEPARATOR) // parse y
				{
				parse(buf+1, y, &ymax, N);
				
				// store or delete x->y
				for( i = 0; i < xmax-1; i++)
					for( j = i+1; j < xmax; j++)
						{
						u = N *(-2 - 3*x[i] - x[i]*x[i] + 2*x[j] + 2*x[i]*N) / 2 ;
						if (delete == 0) // store
							for( k = 0; k < ymax; k++) ++ *( T + u + y[k] );
						else // delete
							for( k = 0; k < ymax; k++) -- *( T + u + y[k] );
						}
				}
				
			else if (*buf == 0) // query
				{
				
				for( i = 0; i < N; i++ ) y[i] = 0;
			
				for( i = 0; i < xmax-1; i++)
					for( j = i+1; j < xmax; j++)
						{
						u = N *(-2 - 3*x[i] - x[i]*x[i] + 2*x[j] + 2*x[i]*N) / 2 ;
						for( k = 0; k < N; k++) y[k] += *( T + u + k);
						}
						
				binarize(y, N, P);
				
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
