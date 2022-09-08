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



/*

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
		
		if ( (*i)-- > s->n || *i < 0 ) // subtracting 1 because internal representation range is 0 to N-1
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
	
	while (	1 )
		{
		printf("Enter data ...\n");

		fgets(inputline, sizeof(inputline), stdin);

		if (! strcmp(inputline, "quit\n"))
			exit(0);
		
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
	
