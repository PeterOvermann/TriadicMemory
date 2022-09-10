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
*/




#define VERSIONMAJOR 1
#define VERSIONMINOR 1

#define NMAX 20000

#define SEPARATOR ','

#include <stdint.h>

#define YTYPE uint32_t 		// combines 16 bits for the index and 16 bits for the counter
#define YSHIFT 16
#define CHUNKSIZE 0
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
	//int sorted[n];
	int* sorted = malloc(n * sizeof(int));
	
	for ( int i=0; i < n; i++ )
		sorted[i] = v[i];
	
	qsort( sorted, n, sizeof(int), cmpfunc);
	
	int rankedmax = sorted[ n - p ];
	
	if(rankedmax == 0)
		rankedmax = 1;
		
	for ( int i=0; i<n; i++) if (v[i] >= rankedmax) printf( "%d ", i+1 );

	free(sorted);
	
	printf( "\n");
	fflush(stdout);
	}


static void print_help()
	{
	printf("sparseassociativememory %d.%d\n\n", VERSIONMAJOR, VERSIONMINOR);
	printf("Sparse distributed memory (SDM) for storing associations x->y of sparse binary hypervectors x and y.\n");
	printf("A hypervector is given by an ordered set of p integers from 1 to n which represent its \"1\" bits.\n");

	printf("The dimension of hypervector x should not exceed n=%d in this implementation. The dimension of y is unlimited.\n", NMAX);


	printf("\n");
	printf("Command line arguments:\n\n");

	printf("sparseassociativememory  	      	(assumes default value n=%d for dimension of x)\n", NMAX);
	printf("sparseassociativememory n        	(uses n as the dimension of x)\n\n");
		
		
	printf("Usage examples:\n\n");
	printf("Store x->y:\n");
	printf("1 20 195 355 371 471 603  814 911 999, 13 29 41 182 590 711 714 773 925 967\n\n");
		
	printf("Recall y for a given x:\n");
	printf("1 20 195 355 371 471 603  814 911 999\n\n");
		
	printf("Print this help text:\n");
	printf("help\n\n");
	
	printf("Show version number:\n");
	printf("version\n\n");

	printf("Terminate process:\n");
	printf("quit\n\n");
	}



int main(int argc, char *argv[])
	{
	int Ny = 1;			// the largest index encoutered so far in y
	double P = 0; 			// average sparse population of inputs y
	int items = 0;			// how many associations have been stored
	
	int chunk = 32;
	
	int N = NMAX;  // max SDR dimension
	
	if (argc < 2)
		{
		print_help();
		exit(1);
		}
		
	sscanf( argv[1], "%d", &N); // x dimension
	
	// ignoring additional command line arguments, so this program is plug-compatible with dyadicmemory
	
	if (N > NMAX)
		{
		printf("vector dimension %d exceeds maximum value of %d\n", N, NMAX);
		exit(1);
		}
	
	int *x = (int*)malloc(N*sizeof(int));
	int *y = (int*)malloc(N*sizeof(int));
	
	YTYPE **T = (YTYPE**) malloc( (1 + N*(N-1)/2) * sizeof(YTYPE*));

	for(int i = 1; i <=  N*(N-1)/2 ; i++)  *(T + i) = 0; // memory initialization 1 .. N(N-1)/2
	
	char inputline[INPUTBUFFER];
	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{
		if ( strcmp(inputline, "quit\n") == 0)
			return 0;

		if ( strcmp(inputline, "version\n") == 0)
			printf("%d.%d\n", VERSIONMAJOR, VERSIONMINOR);
		
		else if ( strcmp(inputline, "help\n") == 0)
			print_help();

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
					items++;
					P = ((items-1)*P + ymax) / items;
				
					// update largest index encountered so far
					if( y[ymax-1] > Ny)
						Ny = y[ymax-1];
					}
				
				// store x->y
				for( int j = 1; j < xmax; j++ )
					for ( int i = 0; i < j; i++ )
						{
						int sort = 0;
						
						int addr = x[i] + x[j]*(x[j]-1) / 2;
						
						if (! T[addr]) // initial allocation of memory chunk
							{
							// allocate twice the average sparse population of y
							int newchunksize = 2 * (int) round(P);
							if (newchunksize > chunk) chunk = newchunksize;
							
							T[addr] = (YTYPE*) malloc(chunk * sizeof(YTYPE));
							
							T[addr][CHUNKSIZE] = chunk; 		// bookkeeping: allocated size
							T[addr][LENGTH] = 0;			// bookkeeping: used size
							}
							
						YTYPE *Y = T[addr];
								
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
									T[addr] = Y = newchunk;
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
			
				for( int j = 1; j < xmax; j++ )
					for ( int i = 0; i < j; i++ )
						{
						int addr = x[i] + x[j]*(x[j]-1) / 2;
						
						YTYPE *Y = T[addr];
						
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
