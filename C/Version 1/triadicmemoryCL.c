/*
triadicmemoryCL.c

Triadic Memory Command Line


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


#define VERSIONMAJOR 1
#define VERSIONMINOR 4


static void print_help()
	{
	printf("triadicmemory %d.%d\n\n", VERSIONMAJOR, VERSIONMINOR);
	printf("Sparse distributed memory for storing triple associations {x,y,z} of sparse binary hypervectors.\n");
	printf("A hypervector of dimension n is given by an ordered set of p integers with values from 1 to n which represent its \"1\" bits.\n");
		
	printf("\n");
	printf("Command line arguments:\n\n");
	printf("triadicmemory n p            (n is the vector dimension, p is the vector's target sparse population)\n\n");
		
		
	printf("Usage examples:\n\n");
	printf("Store {x,y,z}:\n");
	printf("{37 195 355 371 471 603 747 914 943 963, 73 252 418 439 461 469 620 625 902 922, 60 91 94 128 249 517 703 906 962 980}\n\n");
		
	printf("Recall x:\n");
	printf("{_ , 73 252 418 439 461 469 620 625 902 922,  60 91 94 128 249 517 703 906 962 980}\n\n");
		
	printf("Recall y:\n\n");
	printf("{37 195 355 371 471 603 747 914 943 963, _ , 160 91 94 128 249 517 703 906 962 980}\n\n");

	printf("Recall z:\n\n");
	printf("{37 195 355 371 471 603 747 914 943 963, 73 252 418 439 461 469 620 625 902 922, _}\n\n");

	printf("Delete {x,y,z} from memory:\n");
	printf("-{37 195 355 371 471 603 747 914 943 963, 73 252 418 439 461 469 620 625 902 922, 60 91 94 128 249 517 703 906 962 980}\n\n");

	printf("Generate a random vector:\n");
	printf("random\n\n");

	printf("Print this help text:\n");
	printf("help\n\n");
	
	printf("Show version number:\n");
	printf("version\n\n");

	printf("Terminate process:\n");
	printf("quit\n\n");
	}
	
	
static char* parse (char *buf, SDR *s)
	{
	buf = sdr_parse(buf, s);
	
	if (*buf == QUERY && s->p == 0)  { s->p = -1; buf++; while (isspace(*buf)) buf++;}
	
	if (*buf == SEPARATOR) buf++;
	
	return buf;
	}


int main(int argc, char *argv[])
	{
	char *buf, inputline[10000];
	
	if (argc != 3)
		{
		print_help();
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
	
		if (! strcmp(inputline, "help\n"))
			print_help();

		else if (! strcmp(inputline, "random\n"))
			sdr_print(sdr_random(x, P));

		else if ( strcmp(inputline, "version\n") == 0)
			printf("triadicmemory %d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
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
	
