/*

sparseassociativememory.c

A Sparse Distributed Memory for binary hypervectors with dimensions up to 20,000,
based on the Dyadic/Triadic Memory algorithms published in the 2022 paper
https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf

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

This self-contained C implementation is optimized for memory usage, allowing the storage and
retrieval of vectors with dimensions up to 20,000. In contrast, the original reference implementation
(dyadicmemory.c) works with dimensions up to 1,000.

Build: cc -Ofast sparseassociativememory.c -o /usr/local/bin/sparseassociativememory

This command line tool instantiates a new memory instance. It stores heteroassociations x->y of
binary Sparse Distributed Representations (SDR) x and y, and recalls y for a given x.

A binary SDR is given by an ordered set of positive (non-zero) integers which represent
the non-zero positions ("1" bits) of the SDR.

The size (dimension) of an SDR is determined dynamically from user input. The maximum
possible value is N = 20,000. A smaller value of N can be optionally specified as command line argument
in order to lower the default memory footprint.

When recalling an association, the target sparse population P of the output is taken to be the average
population of the y that have been stored in memory.

The algorithm allows for an asymmetric use of the associative memory, where the dimension N and the
sparse population P of x and y have different values.

Usage examples:

Store x->y:
1 20 195 355 371 471 603  814 911 999, 13 29 41 182 590 711 714 773 925 967

Recall y:
1 20 195 355 371 471 603  814

Print version number:
version

*/

#define PRODUCTNAME "sparseassociativememory"
#define VERSIONMAJOR 1
#define VERSIONMINOR 0

#define NMAX 20000

#define SEPARATOR ','

#define YTYPE uint32_t 	// combines 16 bits for the index and 16 bits for the counter
#define YSHIFT 16
#define CHUNKSIZE 0		// first to elements of the allocated array are reserved for bookkeeping
#define LENGTH 1
#define OFFSET 2

#define INPUTBUFFER 10000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


char* parse (char *buf, int *vec, int *len, int n)
	{
	*len = 0; // number of indices parsed
	
	while ( *buf != 0 && *buf != SEPARATOR )
		{
		while (isspace(*buf)) buf++;
		if (! isdigit(*buf) && *buf != '-') break;
		
		int *p = vec + *len;
		
		sscanf( buf, "%d", p);
		
		if ( (*p) > n || *p < 1 )
			{
			printf("input %s out of range\n", buf);
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
	
int ycmp (const void * a, const void * b)
	{
	return  *(YTYPE*)a - *(YTYPE*)b;
	}
	

void binarize (int *v, int n, int p)
	{
	int sorted[n];
	
	for ( int i=0; i < n; i++ )
		sorted[i] = v[i];
	
	qsort( sorted, n, sizeof(int), cmpfunc);
	
	int rankedmax = sorted[ n - p ];
	
	if(rankedmax == 0)
		rankedmax = 1;
		
	for ( int i=0; i<n; i++) if (v[i] >= rankedmax) printf( "%d ", i+1 );
	
	printf( "\n");
	fflush(stdout);
	}


int main(int argc, char *argv[])
	{
	int Ny = 1;				// the largest index encoutered so far in y
	double P = 0; 			// average sparse population of inputs y
	int writecount = 0;		// how many associations have been stored
	
	int chunk = 32;
	
	int N = NMAX;  // max SDR dimension
	
	if (argc == 2) // SDR dimension given as command line argument
		sscanf( argv[1], "%d", &N);
		
	if (argc > 2 || N > NMAX)
		{
		printf("usage: \n");
		printf("sparseassociativememory               dynamic SDR dimension   (n <= %d)\n", NMAX);
		printf("sparseassociativememory <n>           fixed SDR dimension n   (n <= %d)\n", NMAX);
		exit(1);
		}
	
	int *x = (int*)malloc(N*sizeof(int));
	int *y = (int*)malloc(N*sizeof(int));
	
	// limitation: malloc may fail for large n, use virtual memory in this case
	YTYPE **T = (YTYPE**) malloc( (1 + N*(N-1)/2) * sizeof(YTYPE*));

	for(int i = 1; i <=  N*(N-1)/2 ; i++)  *(T + i) = 0; // memory initialization 1 .. N(N-1)/2
	
	char inputline[INPUTBUFFER];
	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{
		if ( strcmp(inputline, "version\n") == 0)
			printf("%s %d.%d\n", PRODUCTNAME, VERSIONMAJOR, VERSIONMINOR);
			
		else // parse x
			{
			int xmax;
			char *buf = parse(inputline, x, &xmax, N);
			
			if (*buf == SEPARATOR) // parse y
				{
				int ymax;
				parse(buf+1, y, &ymax, N);
				
				if (ymax > 0)
					{
					// update average sparsity
					writecount++;
					P = ((writecount-1)*P + ymax) / writecount;
				
					// update largest index encountered so far
					if( y[ymax-1] > Ny)
						Ny = y[ymax-1];
					}
				
				// store x->y
				for( int i = 0; i < xmax-1; i++)
					for( int j = i+1; j < xmax; j++)
						{
						int sort = 0;
						
						int u = (-x[i] * (1 + x[i]) + 2*(x[j] + N*(x[i] - 1)) ) / 2;
						
						if (! T[u]) // initial allocation of memory chunk
							{
							// allocate twice the average sparse population of y
							int newchunksize = 2 * (int) round(P);
							if (newchunksize > chunk) chunk = newchunksize;
							
							T[u] = (YTYPE*) malloc(chunk * sizeof(YTYPE));
							
							T[u][CHUNKSIZE] = chunk; 	// bookkeeping: allocated size
							T[u][LENGTH] = 0;			// bookkeeping: used size
							}
							
						YTYPE *Y = T[u];
								
						int t = OFFSET;
						int len = OFFSET + Y[LENGTH];
						
						for( int k = 0; k < ymax; k++)
							{
							YTYPE pos = y[k] << YSHIFT; // big 16 bits are the index value
							
							// advance t to the value of y[k]
							while(t < len && Y[t] < pos) t++;
							
							// index already there: just need to increment
							if( (Y[t] >> YSHIFT) == y[k]) Y[t] ++;
								
							// new index, temporarily append at the end
							else
								{
								if( t < len) sort = 1;
								
								// need to reallocate memory if array has filled up
								if (Y[LENGTH] + OFFSET == Y[CHUNKSIZE]  )
									{
									YTYPE* newchunk;
									int newsize = Y[CHUNKSIZE] + chunk;
									
									newchunk = (YTYPE*) malloc( newsize * sizeof(YTYPE));
									newchunk[CHUNKSIZE] = newsize; // store the allocated size
									newchunk[LENGTH] = Y[LENGTH];
									for (int i = OFFSET; i < Y[LENGTH] + OFFSET; i++)
										newchunk[i] = Y[i];
									free (Y);
									T[u] = Y = newchunk;
									}
								
								Y[OFFSET + Y[LENGTH]++ ] = pos + 1; // little 16 bits store counter value 1
								}
							}
							
						if (sort) // could speed this up by merging two sorted lists
							qsort( Y+OFFSET, Y[LENGTH], sizeof(YTYPE), ycmp);
							
						}
				}
				
			else if (*buf == 0) // query
				{
				for( int i = 0; i < Ny; i++ ) y[i] = 0; // initialize output vector
			
				for( int i = 0; i < xmax-1; i++)		// iterating over all index pairs in x
					for( int j = i+1; j < xmax; j++)
						{
						YTYPE *Y = T[(-x[i] * (1 + x[i]) + 2*(x[j] + N*(x[i] - 1)) ) / 2];
						
						if (Y)
							for (int h = 0; h < Y[LENGTH]; h++)
								{
								YTYPE e = Y[OFFSET + h ];
								y[(e >> YSHIFT) - 1 ] += e & 0xFFFF; // in y, index values start with 0
								}
						}
						
				binarize(y, Ny, (int)round(P));
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
