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



int main(int argc, char *argv[])
	{
	clock_t start;
        
	int Nx = 1000;		// x dimension
	int Ny = 1000;		// y dimension
	int P  = 10;  		// y sparse population

	
   	DyadicMemory *T = dyadicmemory_new(Nx, Ny, P);
  	
	printf("DyadicMemory capacity and performance tests\n");
	printf("Nx = %d, Ny = %d, P = %d\n\n", Nx, Ny, P);

  	
    	int size = 100000; // test data number of associations
	
	SDR **t1 	= malloc(size * sizeof(SDR*));
	SDR **t2 	= malloc(size * sizeof(SDR*));
	SDR **out 	= malloc(size * sizeof(SDR*));

	for (int i = 0; i < size; i++)
		{
		t1[i] = sdr_new(Nx);
		t2[i] = sdr_new(Ny);
		out[i]= sdr_new(Ny);
		}
		
	for ( int iter = 1; iter <= 10; iter ++)
		{
		printf("iter %.3d | ", iter);

		// create random test data
		printf("%d items | ", size);
		for (int i = 0; i < size; i++)
			{
			t1[i] = sdr_random(t1[i],P);
			t2[i] = sdr_random(t2[i],P);
			}

	
		// store test data
	
		printf("write ");
		start = clock();
		for (int i = 0; i < size; i++)
			
			dyadicmemory_write( T, t1[i], t2[i]);

	
		printf("%.2fs | ", ((double) (clock() - start)) / CLOCKS_PER_SEC);


		// recall test data

		printf("read ");
	
		start = clock();
		int h[size];
		for (int i = 0; i < size; i++)
			dyadicmemory_read ( T, t1[i], out[i] );
		
		printf("%.2fs | ", ((double) (clock() - start)) / CLOCKS_PER_SEC);

		// calculate hamming distances
		for (int i = 0; i < size; i++)
			h[i] = sdr_distance(t2[i], out[i]);
		double mh = 0;
		for (int i = 0; i < size; i++) mh += h[i];
		printf("%f average H dist\n",   mh/size);

		}

	printf("finished\n");
	return 0;
	}
	

