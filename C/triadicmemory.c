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
	
	return v;
	}




// ---------- SDR utility functions ----------





SDR* sdr_random( SDR*s, int p)
	{
	static int *range = 0;
	int n = s->n;
	s->p = p;
	
	if (! range)
		{
		srand((unsigned int)time(NULL));		// make sure this is called only once
		range = malloc(n*sizeof(int));			// integers 0 to n-1
		for (int i = 0; i < n; i++) range[i] = i;
		}
		
	for (int k = 0; k < p; k++) // random selection of p integers in the range 0 to n-1
		{
		int r = rand() % n;
		s->a[k] = range[r];
		int tmp = range[n-1]; range[n-1] = range[r]; range[r] = tmp; // swap selected value to the end
		n--; // values swapped to the end won't get picked again
		}
		
	qsort( s->a, p, sizeof(int), cmpfunc);
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
	
	
	
// ---------- Dyadic Memory -- stores hetero-associations x->y ----------



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
	
	
static int storagelocation(int n, int i, int j)
	{
	return n * (-2 - 3*i - i*i + 2*j + 2*i*n) / 2; // cell in base triangle
	}
	
	
void dyadicmemory_write (DyadicMemory *D, SDR *x, SDR *y)
	{
	for( int i = 0; i < x->p - 1; i++)
		for( int j = i+1; j < x->p; j++)
			{
			int u = storagelocation( D->n, x->a[i], x->a[j]);
			
			for( int k = 0; k < y->p; k++)
				++ *( D->m + u + y->a[k] );
			}
	}
	
	
void dyadicmemory_delete (DyadicMemory *D, SDR *x, SDR *y)
	{
	for( int i = 0; i < x->p - 1; i++)
		for( int j = i+1; j < x->p; j++)
			{
			int u = storagelocation( D->n, x->a[i], x->a[j]);
			for( int k = 0; k < y->p; k++)
				if (*( D->m + u + y->a[k] ) > 0) // test for counter underflow
					-- *( D->m + u + y->a[k] );
			}
	}
	

SDR* dyadicmemory_read (DyadicMemory *D, SDR *x, SDR *y)
	{
	// warning: x and y must point to different SDRs
	
	for( int k = 0; k < D->n; k++ ) y->a[k] = 0;
			
	for( int i = 0; i < x->p-1; i++)
		for( int j = i + 1; j < x->p; j++)
			{
			int u = storagelocation( D->n, x->a[i], x->a[j]);
			for( int k = 0; k < D->n; k++)
				y->a[k] += *( D->m + u + k);
			}
						
	return binarize(y, D->p);
	}

// ---------- Monadic Memory -- stores autoassociations  ----------


MonadicMemory* monadicmemory_new (int n, int p)
	{
	MonadicMemory *M = malloc( sizeof(MonadicMemory));
	
	M->D1 = dyadicmemory_new(n, p);
	M->D2 = dyadicmemory_new(n, p);
	
	M->h = sdr_new(n);	// hidden value
	M->r = sdr_new(n);	// return value
	
	return M;
	}
	
	
SDR* monadicmemory (MonadicMemory *M, SDR *inp)
	{
	
	dyadicmemory_read(M->D1, inp,  M->h);
	dyadicmemory_read(M->D2, M->h, M->r);
	dyadicmemory_read(M->D1, M->r, M->h);
	dyadicmemory_read(M->D2, M->h, M->r);
	
	if (2*sdr_distance(inp, M->r) < M->D1->p)
		return M->r;
		
	M->items++;
	sdr_random (M->r, M->D1->p);
	
	dyadicmemory_write(M->D1, inp,  M->r);
	dyadicmemory_write(M->D2, M->r, inp );
	
	return sdr_set(M->r, inp);
	}
	



// ---------- Triadic Memory -- stores triple associations (x,y,z}  ----------

	
TriadicMemory *triadicmemory_new(int n, int p)
	{
	TriadicMemory *t = malloc(sizeof(TriadicMemory));
	// limitation: malloc may fail for large n, use virtual memory instead in this case
	
	t->m = (TMEMTYPE*) malloc( n*n*n * sizeof(TMEMTYPE));
	t->n = n;
	t->p = p;
	
	for (int i = 0; i < n*n*n; i++)  *(t->m + i) = 0;
	
	return t;
	}
	
void triadicmemory_write (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int N = T->n;
	
	for( int i = 0; i < x->p; i++) for( int j = 0; j < y->p; j++) for( int k = 0; k < z->p; k++)
		++ *( T->m + N*N*x->a[i] + N*y->a[j] + z->a[k] ); // counter overflow is unlikely
	}
	
void triadicmemory_delete (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int N = T->n;
	
	for( int i = 0; i < x->p; i++) for( int j = 0; j < y->p; j++) for( int k = 0; k < z->p; k++)
		if (*( T->m + N*N*x->a[i] + N*y->a[j] + z->a[k] ) > 0) // checking for counter underflow
			-- *( T->m + N*N*x->a[i] + N*y->a[j] + z->a[k] );
	}
	


SDR* triadicmemory_read_x (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int N = T->n;
	
	for( int i = 0; i < N; i++ ) x->a[i] = 0;
	
	for( int j = 0; j < y->p; j++)  for( int k = 0; k < z->p; k++) for( int i = 0; i < N; i++)
		x->a[i] += *( T->m + N*N*i + N*y->a[j] + z->a[k]);
						
	return binarize(x, T->p);
	}

SDR* triadicmemory_read_y (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int N = T->n;
	
	for( int j = 0; j < N; j++ ) y->a[j] = 0;
		
	for( int i = 0; i < x->p; i++) for( int j = 0; j < N; j++) for( int k = 0; k < z->p; k++)
		y->a[j] += *( T->m + N*N*x->a[i] + N*j + z->a[k]);
						
	return binarize(y, T->p);
	}

SDR* triadicmemory_read_z (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int N = T->n;
	
	for( int k = 0; k < N; k++ ) z->a[k] = 0;
	
	for( int i = 0; i < x->p; i++) for( int j = 0; j < y->p; j++) for( int k = 0; k < N; k++)
		z->a[k] += *( T->m + N*N*x->a[i] + N*y->a[j] + k);
						
	return binarize(z, T->p);
	}
	

// ---------- Temporal Memory  ----------


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
	
	
SDR* temporalmemory (TemporalMemory *T, SDR *inp)
	{
	// flush state variables if input is zero -- needed for usage as sequence memory
	if (inp->p == 0)
		{
		T->y->p = T->c->p = T->u->p = T->v->p = T->prediction->p = 0;
		return T->prediction;
		}
	
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
		
	return triadicmemory_read_z (T->M2, sdr_set(T->u, T->x), sdr_set(T->v, T->y), T->prediction);
	// important: the return value is used in the next iteration and should not be changed by
	// the embedding function
	}


// ---------- Command Line Functions ----------



char* sdr_parse (char *buf, SDR *s)
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
