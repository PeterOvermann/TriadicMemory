//
//  triadicmemory.h
//  TriadicMemory
//
//  Created by Peter on 02.07.22.
//

#ifndef triadicmemory_h
#define triadicmemory_h



// uncomment this if building as a library
#define TRIADICMEMORY_LIBRARY




#define TMEMTYPE unsigned char

typedef struct
	{
	TMEMTYPE* m;
	int n, p;
	} TriadicMemory;
	
typedef struct
	{
	int *a, n, p;
	} SDR;
	
SDR *sdr_random (SDR*, int n);				// random generator
SDR *sdr_new (int n);						// SDR constructor
SDR *sdr_set( SDR *x, SDR *y); 				// copy y to x
SDR *sdr_or (SDR*res, SDR *x, SDR *y);		// store bit-wise OR of x and y and res

int  sdr_equal( SDR*x, SDR*y); 				// whether x and y are identical
int  sdr_distance( SDR*x, SDR*y); 			// Hamming distance
int  sdr_overlap( SDR*x, SDR*y); 			// number of common bits

void sdr_print(SDR *s);						// print SDR followed by newline

TriadicMemory *triadicmemory_new(int n, int p);

void triadicmemory_write   (TriadicMemory *, SDR *, SDR *, SDR *);
void triadicmemory_delete  (TriadicMemory *, SDR *, SDR *, SDR *);

SDR* triadicmemory_read_x  (TriadicMemory *, SDR *, SDR *, SDR *);
SDR* triadicmemory_read_y  (TriadicMemory *, SDR *, SDR *, SDR *);
SDR* triadicmemory_read_z  (TriadicMemory *, SDR *, SDR *, SDR *);



#endif /* triadicmemory_h */
