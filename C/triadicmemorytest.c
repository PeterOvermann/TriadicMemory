/*
triadicmemorytest.c

Triadic Memory setup for capacity and performance tests


Copyright (c) 2022-24 Peter Overmann

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
	int N = 1000;		// SDR dimension
	int P = 10;  		// SDR target sparse population

    	int items 		= 100000;
    	int iterations 		= 10;
    	int tridirectional 	= 1;

  	clock_t start;
  	
  	int* h = (int *)malloc(items * sizeof(int)); // stores Hamming distances for test set
 	double meanhammingdistance;
 	
   	TriadicMemory *T = triadicmemory_new(N, P);
   	
   	T->forgetting = 0; // set to 1 to enable forgetting mode
  	
	printf("Triadic Memory performance and capacity test");
	if (T->forgetting)
		printf(" (random forgetting enabled)");
	printf("\n");
  	
	
	SDR **t1 	= malloc(items * sizeof(SDR*));
	SDR **t2 	= malloc(items * sizeof(SDR*));
	SDR **t3 	= malloc(items * sizeof(SDR*));
	SDR **out 	= malloc(items * sizeof(SDR*));

	for (int i = 0; i < items; i++)
		{
		t1[i] = sdr_new(N);
		t2[i] = sdr_new(N);
		t3[i] = sdr_new(N);
		out[i]= sdr_new(N);
		}
		
	for (int iter = 1; iter <= iterations; iter ++)
		{
		printf("| iter %.3d | n=%d | p=%d | %d items | ", iter, N, P, items);

		// create random test data
		for (int i = 0; i < items; i++)
			{
			t1[i] = sdr_random(t1[i],P);
			t2[i] = sdr_random(t2[i],P);
			t3[i] = sdr_random(t3[i],P);
			}

	
		// store test data
	
		printf("write/sec ");
		start = clock();
		for (int i = 0; i < items; i++)
			triadicmemory_write( T, t1[i],t2[i],t3[i]);
	
		PrintOpsPerSecond;

		// recall z

		printf("z read/sec ");
		start = clock();
		
		for (int i = 0; i < items; i++)
			triadicmemory_read_z ( T, t1[i], t2[i], out[i] );
		
		PrintOpsPerSecond;
		
		// calculate hamming distances
		for (int i = 0; i < items; i++) h[i] = sdr_distance(t3[i], out[i]);
		meanhammingdistance = 0; for (int i = 0; i < items; i++) meanhammingdistance += h[i];
		printf("%.3f avg dist | ", meanhammingdistance/items);


		if (! tridirectional) break;

		// recall y

		printf("y read/sec ");
		start = clock();
		
		for (int i = 0; i < items; i++)
			triadicmemory_read_y ( T, t1[i], out[i], t3[i] );
		PrintOpsPerSecond;

		// calculate hamming distances
		for (int i = 0; i < items; i++) h[i] = sdr_distance(t2[i], out[i]);
		meanhammingdistance = 0; for (int i = 0; i < items; i++) meanhammingdistance += h[i];
		printf("%.3f avg dist | ", meanhammingdistance/items);
	
		// recall x

		printf("x read/sec ");
		start = clock();
		
		for (int i = 0; i < items; i++)
			triadicmemory_read_x ( T, out[i], t2[i], t3[i] );
		
		PrintOpsPerSecond;

		// calculate hamming distances
		for (int i = 0; i < items; i++) h[i] = sdr_distance(t1[i], out[i]);
		meanhammingdistance = 0; for (int i = 0; i < items; i++) meanhammingdistance += h[i];
		printf("%.3f avg dist | \n", meanhammingdistance/items);
		}
		
	printf("\n");
		

	printf("\nfinished\n");
	return 0;
	}
	
