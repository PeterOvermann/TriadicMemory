/*
triadicmemory.c

Copyright (c) 2022 Peter Overmann

C-language reference implementation of the Triadic Memory algorithm published in
   https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf
This source file can be compiled as a stand-alone command line program or as a library.

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

Building the command line version: cc -Ofast triadicmemory.c -o /usr/local/bin/triadicmemory

This command line tool instantiates a new memory instance. It can be used
to store triples {x,y,z} of sparse distributed representations (SDRs), and to
recall one part of a triple by specifying the other two parts.
{x,y,_} recalls the third part, {x,_,z} recalls the second part, {_,y,z} recalls
the first part.

An SDR is given by a set of p integers in the range from 1 to n.
Typical values are n = 1000 and p = 10 to 20.

Command line arguments: triadicmemory <n> <p>

Command line usage examples:

Store {x,y,z}:
{37 195 355 371 471 603 747 914 943 963, 73 252 418 439 461 469 620 625 902 922, 60 91 94 128 249 517 703 906 962 980}

Recall x:
{_ , 73 252 418 439 461 469 620 625 902 922,  60 91 94 128 249 517 703 906 962 980}

Recall y:
{37 195 355 371 471 603 747 914 943 963, _ , 160 91 94 128 249 517 703 906 962 980}

Recall z:
{37 195 355 371 471 603 747 914 943 963, 73 252 418 439 461 469 620 625 902 922, _}

Delete {x,y,z}:
-{37 195 355 371 471 603 747 914 943 963, 73 252 418 439 461 469 620 625 902 922, 60 91 94 128 249 517 703 906 962 980}

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

// various SDR utilities:

SDR* sdr_random( SDR*s, int p)
	{
	static int *range = 0;
	int n = s->n;
	s->p = p;
	
	if (! range)
		{
		srand((unsigned int)time(NULL));				// make sure this is only called once
		range = malloc(n*sizeof(int));					// integers 0 to n-1
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
	return (s);
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
	
	return (res);
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
	
void sdr_print(SDR *s)
	{
	for (int r = 0; r < s->p; r++)
		{
		printf("%d", s->a[r] + 1);
		if (r < s->p -1) printf(" ");
		}
	printf("\n"); fflush(stdout);
	}
	

	
TriadicMemory *triadicmemory_new(int n, int p)
	{
	TriadicMemory *t = malloc(sizeof(TriadicMemory));
	// limitation: malloc may fail for large n, use virtual memory instead in this case
	
	t->m = (TMEMTYPE*) malloc( n*n*n * sizeof(TMEMTYPE));
	t->n = n;
	t->p = p;
	
	TMEMTYPE *cube = t->m;
	
	for (int i = 0; i < n*n*n; i++)  *(cube ++) = 0;
	
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
						
	return( binarize(x, T->p) );
	}

SDR* triadicmemory_read_y (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int N = T->n;
	
	for( int j = 0; j < N; j++ ) y->a[j] = 0;
		for( int i = 0; i < x->p; i++) for( int j = 0; j < N; j++) for( int k = 0; k < z->p; k++)
			y->a[j] += *( T->m + N*N*x->a[i] + N*j + z->a[k]);
						
	return( binarize(y, T->p) );
	}

SDR* triadicmemory_read_z (TriadicMemory *T, SDR *x, SDR *y, SDR *z)
	{
	int N = T->n;
	
	for( int k = 0; k < N; k++ ) z->a[k] = 0;
	for( int i = 0; i < x->p; i++) for( int j = 0; j < y->p; j++) for( int k = 0; k < N; k++)
		z->a[k] += *( T->m + N*N*x->a[i] + N*y->a[j] + k);
						
	return( binarize(z, T->p));
	}
	
// may be set in triadicmemory.h
#ifndef TRIADICMEMORY_LIBRARY
	
#define SEPARATOR ','
#define QUERY '_'

#define VERSIONMAJOR 1
#define VERSIONMINOR 2


static char* parse (char *buf, SDR *s)
	{
	int *i;
	s->p = 0;
	
	while ( *buf != 0 && *buf != SEPARATOR && *buf != '}')
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
		
	if (*buf == QUERY && s->p == 0)  { s->p = -1; buf++; while (isspace(*buf)) buf++;}
	
	if (*buf == SEPARATOR) buf++;
	
	return buf;
	}


int main(int argc, char *argv[])
	{
	char *buf, inputline[10000];
	
	if (argc != 3)
		{
		printf("usage: triadicmemory n p\n");
		printf("n is the hypervector dimension, typically 1000\n");
		printf("p is the target sparse population, typically 10\n");
		exit(1);
		}
        
	int N, P;  // SDR dimension and target sparse population, received from command line

    sscanf( argv[1], "%d", &N);
    sscanf( argv[2], "%d", &P);
   
   	TriadicMemory *T = triadicmemory_new(N, P);
    	
	SDR *x = sdr_new(N);
	SDR *y = sdr_new(N);
	SDR *z = sdr_new(N);
	
	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{
		if (! strcmp(inputline, "quit\n"))
			exit(0);
		
		else if (! strcmp(inputline, "random\n"))
			sdr_print(sdr_random(x, P));

		else if ( strcmp(inputline, "version\n") == 0)
			printf("%d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
		else // parse input of the form { 1 2 3, 4 5 6, 7 8 9 }
			{
			int delete = 0;
			buf = inputline;
			
			if (*buf == '-')
				{ delete = 1; ++buf; }
		
			if (*buf != '{')
				{ printf("expecting '{', found %s\n ", inputline); exit(4); }
		
			buf = parse(parse(parse(buf+1, x), y), z);
		
			if( *buf != '}')
				{ printf("expecting '}', found %s\n ", inputline); exit(4); }
		
			if ( x->p >= 0 && y->p >= 0 && z->p >= 0) // write or delete x, y, z
				{
				if (delete == 0) // write
					triadicmemory_write  (T, x, y, z);
				else // delete
					triadicmemory_delete (T, x, y, z);
				}

			else if ( x->p >= 0 && y->p >= 0 && z->p == -1) // read z
				sdr_print( triadicmemory_read_z (T, x, y, z));
				
			else if ( x->p >= 0 && y->p == -1 && z->p >= 0) // read y
				sdr_print( triadicmemory_read_y (T, x, y, z));

			else if ( x->p == -1 && y->p >= 0 && z->p >= 0) // read x
				sdr_print( triadicmemory_read_x (T, x, y, z));

			else
				{ printf("invalid input\n"); exit(3); }
			}
		}
			
	return 0;
	}
	
#endif // TRIADICMEMORY_LIBRARY
