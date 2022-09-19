 
/*
triadicmemory.c

C-language reference implementation of Triadic Memory and related algorithms published in
   https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf

Copyright (c) 2022 Peter Overmann

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "triadicmemory.h"




// ---------- SDR utility functions ----------


static int cmpfunc (const void * a, const void * b)
	{ return  *(int*)a - *(int*)b; }
	
	
// convert an array of non-negative integers v to an SDR x with target sparse population pop
// this is used by dyadic/triadic memory query functions

static SDR* binarize (SDR *x, int *response, int pop)
	{
	int *sorted = (int *)malloc(x->n * sizeof(int)), rankedmax;
	
	for ( int i=0; i < x->n; i++ )
		sorted[i] = response[i];
	
	qsort( sorted, x->n, sizeof(int), cmpfunc);
	
	rankedmax = sorted[ x->n - pop ];
	
	if(rankedmax == 0)
		rankedmax = 1;
		
	x->p = 0;
	for ( int i = 0; i < x->n; i++)
		if (response[i] >= rankedmax)
			x->a[x->p++] = i;

	free(sorted);
	free(response);	//  was calloc'ed by the calling function

	return x;
	}



static void srand_init()
	{
	static int initialized = 0;
	if (! initialized)
		{
		srand((unsigned int)time(NULL)); initialized = 1;
		}
	}
	

SDR* sdr_random( SDR*s, int p) // fill s with p random bits (in place)
	{
	srand_init();
	
	int n = s->n;
	s->p = p;

	int* range = (int*)malloc(n * sizeof(int));

	for (int i = 0; i < n; i++) range[i] = i; // initialize with integers 0 to n-1
	
	for (int k = 0; k < p; k++) // random selection of p integers in the range 0 to n-1
		{
		int r = rand() % n;
		s->a[k] = range[r];
		int tmp = range[n-1]; range[n-1] = range[r]; range[r] = tmp; // swap selected value to the end
		n--; // values swapped to the end won't get picked again
		}
	
	qsort( s->a, p, sizeof(int), cmpfunc);

	free(range);

	return s;
	}
	
// add noise to an SDR, operating in place
// bits > 0 : add bits (salt)
// bits < 0 : remove bits (pepper)

SDR* sdr_noise( SDR*s, int bits)
	{
	SDR* t = sdr_new(s->n); sdr_set(t,s);
	
	if (bits >= 0) // add bits (salt noise)
		{
		// add random SDR r
		// less than the desired number of bits will be added if r overlaps with s
		
		SDR* r = sdr_new(s->n);
		sdr_or(s, t, sdr_random(r, bits));
		sdr_delete(r);
		}
		
	else	{
		// remove bits
		srand_init();

		s->p += bits;
		if (s->p < 0) s->p = 0;
		
		for (int k = 0; k < s->p; k++) // random selection of s->p integers from SDR t
			{
			int r = rand() % t->p;
			s->a[k] = t->a[r];
			int tmp = t->a[t->p-1]; t->a[t->p-1] = t->a[r]; t->a[r] = tmp; // swap selected value to the end
			t->p--; // values swapped to the end won't get picked again
			}

		qsort( s->a, s->p, sizeof(int), cmpfunc);
		}
	
	sdr_delete(t);
	return s;
	}
	
	
	
SDR *sdr_new(int n)
	{
	SDR *s = malloc(sizeof(SDR));
	s->a = malloc(n*sizeof(int));
	s->n = n;
	s->p = 0;
	return s;
	}
	
void sdr_delete(SDR *s)
	{
	free(s->a);
	free(s);
	}
	
	
	
SDR *sdr_set( SDR *x, SDR *y) // copy y to x
	{
	// x and y need to have the same dimension n
	for (int i = 0; i < y->p; i++)
		x->a[i] = y->a[i];
	x->p = y->p;
	return x;
	}
	
	
SDR *sdr_rotateright( SDR *x ) // shift x right by one bit, operating in place
	{
	if (!x->p) return x;
	
	if (x->a[x->p - 1] < x->n - 1)
		for (int i = 0; i < x->p; i++)
			x->a[i] ++;
			
	else // x->a[x->p -1] == x->n - 1, need to shift indices
		{
		for (int i = x->p - 1; i > 0; i--)
			x->a[i] = x->a[i-1] + 1;
		x->a[0] = 0;
		}
	
	return x;
	}

	
SDR *sdr_rotateleft( SDR *x ) // shift x left by one bit, operating in place
	{
	if (!x->p) return x;
	
	if (x->a[0] > 0)
		for (int i = 0; i < x->p; i++)
			x->a[i] --;
			
	else // x->a[0] == 0, need to shift indices
		{
		for (int i = 0; i < x->p - 1; i++)
			x->a[i] = x->a[i+1] - 1;
		x->a[x->p - 1] = x->n - 1  ;
		}
	
	return x;
	}

	
	
	
SDR *sdr_or (SDR*res, SDR *x, SDR *y)
	{
	// calculates the logical OR of x and y, stores the result in res
	// x, y and res need to have the same dimension n
	res->p = 0;
	
	int i = 0, j = 0;
	
	while (i < x->p || j < y->p )
		{
		if (i == x->p) while (j < y->p)
			res->a[ res->p++] = y->a[j++];
				
		else if (j == y->p) while (i < x->p)
			res->a[ res->p++] = x->a[i++];
		
		else if (x->a[i] < y->a[j] )
			res->a[ res->p++] = x->a[i++];
			
		else if (x->a[i] > y->a[j] )
			res->a[ res->p++] = y->a[j++];
		
		else // x->a[i] == y->a[j]
			{ res->a[ res->p++] = x->a[i]; i++; j++; }
			
		}
	
	return res;
	}
	
	
int sdr_equal( SDR*x, SDR*y) // test if x and y are identical
	{
	int i = 0;
	
	if ( x->p != y->p) return 0;
	
	for (i = 0; i < x->p; i++)
		if (x->a[i] != y->a[i]) return 0;
		
	return 1; // same
	}
	
	
int sdr_distance( SDR*x, SDR*y) // Hamming distance
	{
	int i = 0, j = 0, h = x->p + y->p;
	
	while (i < x->p && j < y->p )
		{
		if (x->a[i] == y->a[j])
			{ h -= 2; i++; j++; }
		else if (x->a[i] < y->a[j]) ++i;
		else ++j;
		}
	
	return h;
	}
	
	
int sdr_overlap( SDR*x, SDR*y) // number of common bits
	{
	int i = 0, j = 0, overlap = 0;
	
	while (i < x->p && j < y->p )
		{
		if (x->a[i] == y->a[j])
			{ ++overlap; i++; j++; }
		else if (x->a[i] < y->a[j]) ++i;
		else ++j;
		}
	
	return overlap;
	}
	
	
// print SDR with positions from 1 to N (representation used by command line tools)
void sdr_print(SDR *s)
	{
	for (int r = 0; r < s->p; r++)
		{
		printf("%d", s->a[r] + 1);  // adding 1 to the internal representation
		if (r < s->p -1) printf(" ");
		}
	printf("\n"); fflush(stdout);
	}
	
// print SDR with positions from 0 to N-1 (the internal representation)
void sdr_print0(SDR *s)
	{
	for (int r = 0; r < s->p; r++)
		{
		printf("%d", s->a[r] );
		if (r < s->p -1) printf(" ");
		}
	printf("\n"); fflush(stdout);
	}
	

#define bit_set(a,i)     a[(i)/8]  |=  (1u << (i)%8)
#define bit_clear(a,i)   a[(i)/8]  &= ~(1u << (i)%8)
#define bit_test(a,i)    a[(i)/8]  &   (1u << (i)%8)  ? 1 : 0
	
	
// ---------- Dyadic Memory -- stores hetero-associations x->y ----------


DyadicMemory *dyadicmemory_new(int nx, int ny, int p)
	{
	DyadicMemory *D = malloc(sizeof(DyadicMemory));
	
	D->nx 		= nx;	// dimension of nx
	D->ny 		= ny;	// dimension of ny
	D->p  		= p; 	// sparsity target for y
	
	// allocate and initialize nx(nx-1)/2 bit-pair addresses for x
	// the storage arrays for each address will be allocated and initialized as needed

	D->C = (byte**) calloc( (1 + D->nx*(D->nx-1)/2), sizeof(byte*));
	
	return D;
	}
	
	
	
void dyadicmemory_write (DyadicMemory *D, SDR *x, SDR *y)
	{
	if (y->p == 0) return;
					
	for (int j = 1; j < x->p; j++ ) for (int i = 0; i < j; i++ )
		{
		int addr = x->a[i] + x->a[j]*(x->a[j]-1) / 2;
		
		// lazy allocation of array for y
		if (! D->C[addr])
			D->C[addr] = (byte*) calloc( (D->ny+7)/8, 1);

		byte *Y = D->C[addr];

		for (int k = 0; k < y->p; k++)
			bit_set (Y, (unsigned int)y->a[k]);
										
		}
	}
	
	

SDR* dyadicmemory_read (DyadicMemory *D, SDR *x, SDR *y)
	{
	int* response = (int*)calloc(D->ny, sizeof(int));

	for (int j = 1; j < x->p; j++ ) for ( int i = 0; i < j; i++ )
		{
		byte *Y = D->C[x->a[i] + x->a[j]*(x->a[j]-1) / 2];

		if (Y) for (unsigned int k = 0; k < D->ny; k++)
			response[k] += bit_test(Y, k);

		}
						
	return binarize(y, response, D->p);
	}
	


// ---------- Triadic Memory -- stores triple associations (x,y,z}  ----------


TriadicMemory *triadicmemory_new(int n, int p)
	{
	return triadicmemory_new3 (n, p, n, p, n, p);
	}

TriadicMemory *triadicmemory_new3 (int nx, int px, int ny, int py, int nz, int pz)
	{
	srand_init();
	
	TriadicMemory *T = malloc(sizeof(TriadicMemory));
		
	T->nx = nx;
	T->ny = ny;
	T->nz = nz;
	T->px = px;
	T->py = py;
	T->pz = pz;

	T->forgetting = 0; // forgetting is an experimental feature, disabled by default
	
	// allocate and initialize the entire storage cube, 1 bit per location
	// limitation: malloc may fail for large n, use virtual memory instead in this case
		
	T->C = (byte*) calloc( (T->nx * T->ny * T->nz + 7) / 8, 1);
	
	return T;
	}
	
void triadicmemory_write (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int Qx = T->ny * T->nz, Qy = T->nz;

	// original triadic memory write algorithm, modified to use 1-bit address locations

	for (int i = 0; i < x->p; i++) for (int j = 0; j < y->p; j++) for (int k = 0; k < z->p; k++)
		{
		unsigned int b = Qx * x->a[i] + Qy * y->a[j] + z->a[k];
		bit_set(T->C, b);
		}

	
	// the following is not part of the original triadic memory algorithm
	// random forgetting, realized by decrementing the same number of memory locations (but not below zero)
	// this has no measurable effect for an almost empty memory
	// disabled by default
	
	if (T->forgetting)
		{
		int memsize = T->nx * T->ny * T->nz;
		for (int i = 0; i < x->p * y->p * z->p; i++)
			{
			unsigned int b = rand() % memsize;
			bit_clear(T->C, b);
			}
		}
	}
	
		

// triadic memory read algorithm, modified for 1-bit storage locations

// there is a query function for x, y, and z, respectively
// weights are collected into an response vector
// the function binarize converts the response vector to a binary SDR with the specified target population
// note that the result can have a population less or greater than specified


SDR* triadicmemory_read_x (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int* response = (int*)calloc(T->nx, sizeof(int));
	int Qx = T->ny * T->nz, Qy = T->nz;

	for (int j = 0; j < y->p; j++) for (int k = 0; k < z->p; k++)
		{
		unsigned int addr = Qy * y->a[j] + z->a[k];
		
		for (int i = 0; i < T->nx; i++)
			{
			unsigned int b = addr + Qx*i;
			response[i] += bit_test(T->C, b);
			}
		}

	return binarize(x, response, T->px);
	}


SDR* triadicmemory_read_y (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int* response = (int*)calloc(T->ny, sizeof(int));
	int Qx = T->ny * T->nz, Qy = T->nz;
		
	for ( int i = 0; i < x->p; i++) for ( int k = 0; k < z->p; k++)
		{
		unsigned int addr = Qx * x->a[i] + z->a[k];
		
		for ( int j = 0; j < T->ny; j++)
			{
			unsigned int b = addr + Qy*j;
			response[j] += bit_test(T->C, b );
			}
		}

	return binarize(y, response, T->py);
	}



SDR* triadicmemory_read_z (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int* response = (int*)calloc(T->nz, sizeof(int));
	int Qx = T->ny * T->nz, Qy = T->nz;
	
	for (int i = 0; i < x->p; i++) for (int j = 0; j < y->p; j++)
		{
		unsigned int addr = Qx * x->a[i] + Qy * y->a[j];

		for (int k = 0; k < T->nz; k++)
			{
			unsigned int b = addr + k;
			response[k] += bit_test(T->C, b);
			}
		}
		
	return binarize(z, response, T->pz);
	}
	


// ---------- Command Line Functions ----------



char* sdr_parse (char *buf, SDR *s)
	{
	int *i;
	s->p = 0;
	
	while ( *buf  && *buf != SEPARATOR )
		{
		while (isspace(*buf)) buf++;
		if (! isdigit(*buf)) break;
		
		i = s->a + s->p;
		sscanf( buf, "%d", i);
		
		if ( (*i)-- > s->n || *i < 0 ) // subtracting 1 to convert to C convention
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
