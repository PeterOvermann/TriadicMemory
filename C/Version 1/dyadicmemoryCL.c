/*
dyadicmemoryCL.c

Dyadic Memory Command Line wrapper


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

#include "triadicmemory.h"


#define VERSIONMAJOR 1
#define VERSIONMINOR 4

#define INPUTBUFFER 10000


static void print_help()
	{
	printf("dyadicmemory %d.%d\n\n", VERSIONMAJOR, VERSIONMINOR);
	printf("Sparse distributed memory (SDM) for storing associations x->y of sparse binary hypervectors x and y.\n");
	printf("A hypervector of dimension n is given by an ordered set of p integers with values from 1 to n which represent its \"1\" bits.\n");
		
	printf("\n");
	printf("Command line arguments:\n\n");
	printf("dyadicmemory n p             (n is the dimension of x and y, p is the target sparse population of y)\n");
	printf("dyadicmemory nx ny p         (nx and ny are the dimensions of x and y, p is the target sparse population of y)\n\n");
		
		
	printf("Usage examples:\n\n");
	printf("Store x->y:\n");
	printf("1 20 195 355 371 471 603  814 911 999, 13 29 41 182 590 711 714 773 925 967\n\n");
		
	printf("Recall y for a given x:\n");
	printf("1 20 195 355 371 471 603  814 911 999\n\n");
		
	printf("Delete x->y from memory:\n");
	printf("- 1 20 195 355 371 471 603  814 911 999, 13 29 41 182 590 711 714 773 925 967\n\n");
		
	printf("Print this help text:\n");
	printf("help\n\n");
	
	printf("Show version number:\n");
	printf("version\n\n");

	printf("Terminate process:\n");
	printf("quit\n\n");
	}
	
	

int main(int argc, char *argv[])
	{
	char *buf, inputline[INPUTBUFFER];
	
	int Nx, Ny, P;  // vector dimension and target sparse population
	
	if (argc == 3)
		{
		sscanf( argv[1], "%d", &Nx); Ny = Nx;
		sscanf( argv[2], "%d", &P);
		}
	
	else if (argc == 4)
		{
		sscanf( argv[1], "%d", &Nx);
		sscanf( argv[2], "%d", &Ny);
		sscanf( argv[3], "%d", &P);
		}
		
	else	{
		print_help();
		exit(1);
		}
		
	if (Nx > NMAX)
		{
		printf("vector dimension %d exceeds maximum value %d\n", Nx, NMAX);
		exit(20);
		}
    
	SDR *x = sdr_new(Nx);
	SDR *y = sdr_new(Ny);
	
	DyadicMemory *D = dyadicmemory_new(Nx, Ny, P);

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
			int delete = 0;
			
			if (*inputline == '-')
				delete = 1;
			
			buf = sdr_parse(inputline + delete, x);
			
			if (*buf == SEPARATOR) // parse y
				{
				sdr_parse(buf+1, y);
				
				// store or delete x->y
				if (delete)
					dyadicmemory_delete (D, x,y);
				else
					dyadicmemory_write (D, x,y);
				}
				
			else if (*buf == 0) // query
				{
				dyadicmemory_read (D, x, y);
				sdr_print(y);
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
