/*
dyadicmemorytest.c

Dyadic Memory setup for capacity and performance tests


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


#define PrintOpsPerSecond printf("%d | ", (int)round((double)items * CLOCKS_PER_SEC / ((double) (clock() - start))) )

int main(int argc, char *argv[])
	{
	int Nx = 1000;		// x dimension
	int Ny = 1000;		// y dimension
	int P  = 10;  		// y sparse population

    	int items = 100000; 	// number of items in test data
    	int iterations = 10;

	clock_t start;

  	int* h = (int *)malloc(items * sizeof(int)); // stores Hamming distances for test set
	double meanhammingdistance;
	
   	DyadicMemory *T = dyadicmemory_new(Nx, Ny, P);
  	
	printf("Dyadic Memory performance and capacity test\n");
	
	SDR **t1 	= malloc(items * sizeof(SDR*));
	SDR **t2 	= malloc(items * sizeof(SDR*));
	SDR **out 	= malloc(items * sizeof(SDR*));

	for (int i = 0; i < items; i++)
		{
		t1[i] = sdr_new(Nx);
		t2[i] = sdr_new(Ny);
		out[i]= sdr_new(Ny);
		}
		
	for ( int iter = 1; iter <= iterations; iter ++)
		{
		printf("| iter %.3d | nx=%d | ny=%d | p=%d | %d items | write/sec ", iter, Nx, Ny, P, items);

		// create random test data
		
		for (int i = 0; i < items; i++)
			{
			t1[i] = sdr_random(t1[i],P);
			t2[i] = sdr_random(t2[i],P);
			}

		// store test data
	
		start = clock();
		for (int i = 0; i < items; i++)
			dyadicmemory_write( T, t1[i], t2[i]);
	
		PrintOpsPerSecond;

		// recall test data

		printf("read/sec ");
		start = clock();
		for (int i = 0; i < items; i++)
			dyadicmemory_read ( T, t1[i], out[i] );
		
		PrintOpsPerSecond;

		// calculate hamming distances
		for (int i = 0; i < items; i++)
			h[i] = sdr_distance(t2[i], out[i]);

		meanhammingdistance = 0; for (int i = 0; i < items; i++) meanhammingdistance += h[i];
		printf("%.3f avg dist |\n",   meanhammingdistance/items);
		}

	printf("finished\n");
	return 0;
	}
	

