/*
dyadicmemoryCL.c

Dyadic Memory Command Line


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
Building the command line tool: cc -Ofast dyadicmemoryCL.c triadicmemory.c -o /usr/local/bin/dyadicmemory

This command line tool instantiates a new memory instance.

It stores heteroassociations x->y of Sparse Distributed Representations (SDR) x and y,
and recalls y for a given x.

An SDR is given by a set of p integers in the range from 1 to n, representing the non-zero positions of the SDR.
(Note that the internal representation uses integers from 0 to n-1.)

Typical values are n = 1000 and p = 10

Command line usage: dyadicmemory 1000 10


Usage:

Store x->y:
1 20 195 355 371 471 603  814 911 999, 13 29 41 182 590 711 714 773 925 967

Recall y:
1 20 195 355 371 471 603  814 911 999

Delete x->y:
- 1 20 195 355 371 471 603  814 911 999, 13 29 41 182 590 711 714 773 925 967

Terminate process:
quit

Print version number:
version

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "triadicmemory.h"


#define VERSIONMAJOR 1
#define VERSIONMINOR 2

#define SEPARATOR ','

#define INPUTBUFFER 10000


static char* parse (char *buf, SDR *s)
	{
	int *i;
	s->p = 0;
	
	while ( *buf  && *buf != SEPARATOR )
		{
		while (isspace(*buf)) buf++;
		if (! isdigit(*buf)) break;
		
		i = s->a + s->p;
		sscanf( buf, "%d", i);
		
		if ( (*i)-- > s->n || *i < 0 ) // subtracting 1 because internal representation range is 0 to N-1
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


int main(int argc, char *argv[])
	{
	char *buf, inputline[INPUTBUFFER];
	
	int N, P;  // vector dimension and target sparse population
	
	if (argc != 3)
		{
		printf("usage: dyadicmemory n p\n");
		printf("n is the hypervector dimension, typically 1000\n");
		printf("p is the target sparse population, typically 10\n");
		exit(1);
		}
        
    sscanf( argv[1], "%d", &N);
    sscanf( argv[2], "%d", &P);
    
    SDR *x = sdr_new(N);
	SDR *y = sdr_new(N);
	
	DyadicMemory *D = dyadicmemory_new(N, P);

	while (	fgets(inputline, sizeof(inputline), stdin) != NULL)
		{
		if ( strcmp(inputline, "quit\n") == 0)
			return 0;
			
		else if ( strcmp(inputline, "version\n") == 0)
			printf("dyadicmemory %d.%d\n", VERSIONMAJOR, VERSIONMINOR);
			
		else // parse x
			{
			int delete = 0;
			
			if (*inputline == '-')
				delete = 1;
			
			buf = parse(inputline + delete, x);
			
			if (*buf == SEPARATOR) // parse y
				{
				parse(buf+1, y);
				
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
