/*

triadicmemory.c

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


This is the reference implementation of the Triadic Memory algorithm as command line tool.

Build: cc -Ofast triadicmemory.c -o /usr/local/bin/triadicmemory

This command line tool instantiates a new memory instance. It can be used
to store triples {x,y,z} of sparse distributed representations (SDRs), and to
recall one part of a triple by specifying the other two parts.
{x,y,_} recalls the third part, {x,_,z} recalls the second part, {_,y,z} recalls
the first part.

An SDR is given by a set of p integers in the range from 1 to n
Typical values are n = 1000 and p = 10
Command line usage: triadicmemory n p


Examples:

Store {x,y,z}:
{37 195 355 371 471 603 747 914 943 963, 73 252 418 439 461 469 620 625 902 922, 60 91 94 128 249 517 703 906 962 980}

Recall x:
{_ , 73 252 418 439 461 469 620 625 902 922,  60 91 94 128 249 517 703 906 962 980}

Recall y:
{37 195 355 371 471 603 747 914 943 963, _ , 160 91 94 128 249 517 703 906 962 980}

Recall z:
{37 195 355 371 471 603 747 914 943 963, 73 252 418 439 461 469 620 625 902 922, _}

Terminate process:
quit

Print version number:
version

*/


#define VERSIONMAJOR 1
#define VERSIONMINOR 0

#define SEPARATOR ','
#define QUERY '_'
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
	
	while ( *buf != 0 && *buf != SEPARATOR && *buf != '}')
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
		
	if (*buf == QUERY && *len == 0)  { *len = -1; buf++; while (isspace(*buf)) buf++;}
	
	if (*buf == SEPARATOR) buf++;
	
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
	int *x, *y, *z;
	int xmax, ymax, zmax;
	
	int i, j, k;
	int N, P;  // vector dimension and target sparse population
	
	MEMORYTYPE *cube;
	
	if (argc != 3)
		{
		printf("usage: triadicmemory n p\n");
		printf("n is the hypervector dimension, typically 1000\n");
		printf("p is the target sparse population, typically 10\n");
		exit(1);
		}
        
    sscanf( argv[1], "%d", &N);
    sscanf( argv[2], "%d", &P);
   
    	
	x = (int*)malloc(N*sizeof(int));
	y = (int*)malloc(N*sizeof(int));
	z = (int*)malloc(N*sizeof(int));
	
	cube = (MEMORYTYPE*) malloc( N*N*N * sizeof(MEMORYTYPE)); // limitation: malloc may fail for large n, use virtual memory in this case

	for(i = 0; i < N*N*N; i++)  *(cube + i) = 0;

	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{

		if ( strcmp(inputline, "quit\n") == 0)
			return 0;
			
		else if ( strcmp(inputline, "version\n") == 0)
			printf("%d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
		else // parse triple
			{
		
			if (*inputline != '{')
				{
				printf("expecting triple of the form {x,y,z}, found %s\n ", inputline); exit(4);
				}
		
			buf = parse(parse(parse(inputline+1, x, &xmax, N), y, &ymax, N), z, &zmax, N);
		
			if( *buf != '}')
				{
				printf("expecting triple of the form {x,y,z}, found %s\n ", inputline); exit(4);
				}
		
			if ( xmax >= 0 && ymax >= 0 && zmax >= 0) // store x, y, z
				for( i = 0; i < xmax; i++) for( j = 0; j < ymax; j++) for( k = 0; k < zmax; k++)
					++ *( cube + N*N*x[i] + N*y[j] + z[k] ); // counter overflow is unlikely
	
			else if ( xmax >= 0 && ymax >= 0 && zmax == -1) // recall z
				{
				for( k = 0; k < N; k++ ) z[k] = 0;
			
				for( i = 0; i < xmax; i++) for( j = 0; j < ymax; j++) for( k = 0; k < N; k++)
					z[k] += *( cube + N*N*x[i] + N*y[j] + k);
						
				binarize(z, N, P);
				}

			else if ( xmax >= 0 && ymax == -1 && zmax >= 0) // recall y
				{
				for( j = 0; j < N; j++ ) y[j] = 0;
			
				for( i = 0; i < xmax; i++) for( j = 0; j < N; j++)  for( k = 0; k < zmax; k++)
					y[j] += *( cube + N*N*x[i] + N*j + z[k]);
						
				binarize(y, N, P);
				}
			
			else if ( xmax == -1 && ymax >= 0 && zmax >= 0) // recall x
				{
				for( i = 0; i < N; i++ ) x[i] = 0;
			
				for( j = 0; j < ymax; j++)  for( k = 0; k < zmax; k++) for( i = 0; i < N; i++)
					x[i] += *( cube + N*N*i + N*y[j] + z[k]);
						
				binarize(x, N, P);
				}
		
			else
				{
				printf("invalid input\n"); exit(3);
				}
			
			}
		

		}
		
	
	return 0;
	}
