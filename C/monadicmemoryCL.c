/*
monadicmemory.c
Monadic Memory Command Line

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


/*

Monadic Memory stores autoassociations of sparse distributed representations (SDRs).

It can be used as a clean-up memory or as a clustering/pooling algorithm.

This command line tool instantiates a new instance of a Monadic Memory.

An SDR is given by a set of p integers in the range from 1 to n.
Typical values are n = 1000 and p = 10 to 20.

Command line arguments: monadicmemory <n> <p>

Command line usage:
29 129 238 356 451 457 589 620 657 758

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



#define VERSIONMAJOR 1
#define VERSIONMINOR 0



int main(int argc, char *argv[])
	{
	char inputline[10000];
	
	if (argc != 3)
		{
		printf("usage: monadicmemory <n> <p>\n");
		printf("n is the hypervector dimension    (typical value 1000)\n");
		printf("p is the target sparse population (typical value 10 to 20)\n");
		exit(1);
		}
        
	int N, P;  // SDR dimension and target sparse population, received from command line

    sscanf( argv[1], "%d", &N);
    sscanf( argv[2], "%d", &P);
    
    MonadicMemory *M = monadicmemory_new (N, P);
   
	SDR *inp = sdr_new(N);
	
	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{
		if (! strcmp(inputline, "quit\n"))
			exit(0);

		if (! strcmp(inputline, "random\n"))
			sdr_print(sdr_random(inp, P));

		else if ( strcmp(inputline, "version\n") == 0)
			printf("monadicmemory %d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
		else // parse input SDR
			{
			if (* sdr_parse(inputline, inp) )
				{
				printf("unexpected input: %s", inputline);
				exit(5);
				}
			sdr_print( monadicmemory(M, inp));
			}
		}
			
	return 0;
	}
	
