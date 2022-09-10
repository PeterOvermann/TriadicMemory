/*
triadicmemorytest.c

Triadic Memory setup for capacity and performance tests


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



int main(int argc, char *argv[])
	{
	
	clock_t start;
        
	int N = 1000;		// SDR dimension
	int P = 10;  		// SDR target sparse population

	sscanf( argv[1], "%d", &N);
	sscanf( argv[2], "%d", &P);
   
   	TriadicMemory *T = triadicmemory_new(N, P);
   	
   	T->forgetting = 1;
  	
	printf("Triadic Memory capacity and performance tests\n");
	printf("The recall error is given as the average Hamming distance\n");
	printf("Forgetting = %d\n", T->forgetting);
	
	printf("N = %d, P = %d\n\n", N, P);

  	
    	int size = 100000;
	
	SDR **t1 	= malloc(size * sizeof(SDR*));
	SDR **t2 	= malloc(size * sizeof(SDR*));
	SDR **t3 	= malloc(size * sizeof(SDR*));
	SDR **out 	= malloc(size * sizeof(SDR*));

	for (int i = 0; i < size; i++)
		{
		t1[i] = sdr_new(N);
		t2[i] = sdr_new(N);
		t3[i] = sdr_new(N);
		out[i]= sdr_new(N);
		}
		
	for ( int iter = 1; iter <= 30; iter ++)
		{
		printf("iter %.3d | ", iter);

		// create random test data
		printf("%d triples | ", size);
		for (int i = 0; i < size; i++)
			{
			t1[i] = sdr_random(t1[i],P);
			t2[i] = sdr_random(t2[i],P);
			t3[i] = sdr_random(t3[i],P);
			}

	
		// store test data
	
		printf("write ");
		start = clock();
		for (int i = 0; i < size; i++)
			
			triadicmemory_write( T, t1[i],t2[i],t3[i]);

	
		printf("%.2fs | ", ((double) (clock() - start)) / CLOCKS_PER_SEC);



		//int h[size];
		int* h = (int*)malloc(size * sizeof(int));

		double mh;

		// recall z

		printf("read z ");
	
		start = clock();
		
		for (int i = 0; i < size; i++)
			triadicmemory_read_z ( T, t1[i], t2[i], out[i] );
		
		printf("%.2fs | ", ((double) (clock() - start)) / CLOCKS_PER_SEC);

		// calculate hamming distances
		for (int i = 0; i < size; i++) h[i] = sdr_distance(t3[i], out[i]);
		mh = 0; for (int i = 0; i < size; i++) mh += h[i];
		printf("%.3f err | ", mh/size);


		if (0) // also recall y and x
			{


			// recall y

			printf("read y ");
	
			start = clock();
		
			for (int i = 0; i < size; i++)
				triadicmemory_read_y ( T, t1[i], out[i], t3[i] );
		
			printf("%.2fs | ", ((double) (clock() - start)) / CLOCKS_PER_SEC);

			// calculate hamming distances
			for (int i = 0; i < size; i++) h[i] = sdr_distance(t2[i], out[i]);
			mh = 0; for (int i = 0; i < size; i++) mh += h[i];
			printf("%.3f err | ", mh/size);
	
			// recall x

			printf("read x ");
	
			start = clock();
		
			for (int i = 0; i < size; i++)
				triadicmemory_read_x ( T, out[i], t2[i], t3[i] );
		
			printf("%.2fs | ", ((double) (clock() - start)) / CLOCKS_PER_SEC);

			// calculate hamming distances
			for (int i = 0; i < size; i++) h[i] = sdr_distance(t1[i], out[i]);
			mh = 0; for (int i = 0; i < size; i++) mh += h[i];
			printf("%.3f err", mh/size);

			}

		free(h);

		printf("\n");
		}

	printf("finished\n");
	return 0;
	}
	
