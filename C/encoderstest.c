#include <stdio.h>
#include <stdlib.h>

#include "triadicmemory.h"
#include "encoders.h"

int main(int argc, char* argv[]) 
{
	/*
		Test with n = 10000, p = 10, xmin = -10.0, xmax = 10.0
	*/

	SDR* sdr = sdr_new(10000);

	for (double x = -10.0; x <= 10.0; x += 0.01) {

		printf("value before encoding to sdr: %.2f\nsdr: ", x);
		
		Real2SDR(sdr, x, 10000, 10, -10.0, 10.0);

		sdr_print(sdr);

		printf("value after decoding from sdr: %f\n\n", SDR2Real(sdr, 10000, 10, -10.0, 10.0));
	}

	sdr_delete(sdr);

	return 0;
}