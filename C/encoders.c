#include <stdlib.h>
#include <math.h>

#include "triadicmemory.h"
#include "encoders.h"

int cmpfunc(const void* a, const void* b)
{
	return  *(int*)a - *(int*)b;
}

double Round(double x, double a)
{
	if (x / a - (int)(x / a) < 0.5)
		return floor(x / a) * a;
	else
		return ceil(x / a) * a;
}

/*
* Floor[x], gives the greatest integer less than or equal to x.
* SparseArray[data, { d1,d2,... }], yields a sparse array representing a d1×d2×... array.
* RotateRight[expr, n], cycles the elements in expr n positions to the right.
* PadRight[list, n], makes a list of length n by padding list with zeros on the right.
* Table[expr, n], generates a list of n copies of expr.
*
* Flatten[list], flattens out nested lists.
* If[condition,t,f], gives t if condition evaluates to True, and f if it evaluates to False.
* N[expr], gives the numerical value of expr.
* Round[x,a], rounds to the nearest multiple of a.
* Mean[list], gives the statistical mean of the elements in list.
* Boole[expr], yields 1 if expr is True and 0 if it is False.
* OddQ[expr], gives True if expr is an odd integer, and False otherwise.
*/

/*
Mathematica: 
------------
Real2SDR[x_, { xmin_, xmax_ }, { n_Integer, p_Integer }] :=
	Module[{m},
		m = Floor[(x - xmin) / (xmax - xmin) * (n - p)];

		SparseArray[
			RotateRight[
				PadRight[
					Table[1, p], n
				], m
			], { n }
		]
	];
*/
SDR* Real2SDR(SDR* s, double x, int n, int p, double xmin, double xmax) 
{
	s->p = p;

	int m = (int)floor((x - xmin) / (xmax - xmin) * ((double)n - (double)p));

	for (int i = 0; i < p; i++) {
		s->a[i] = i + m;
	}

	qsort(s->a, p, sizeof(int), cmpfunc);

	return s;
}

/*
Mathematica:
------------
SDR2Real[x_SparseArray, { xmin_, xmax_ }, { n_Integer, p_Integer }] :=
	Module[{a},
		a = Flatten[x["NonzeroPositions"]];

		If[Length[a] == 0,
			0,
			N[
				Round[
					(Mean[a] - (p + 1) / 2) / (n - p), 1 / (n - p - Boole[OddQ[n - p]])
				] * (xmax - xmin) + xmin
			]
		]
	];
*/
double SDR2Real(SDR* s, int n, int p, double xmin, double xmax) 
{
	int sum = 0;

	for (int i = 0; i < p; i++) {
		if (s->a[i] != 0) break;
		if (i == p - 1) return 0.0;
	}

	for (int i = 0; i < p; i++) {
		sum += s->a[i];
	}

	double Mean = (double)sum / (double)p;

	double Boole = (n - p) % 2 == 0 ? 0.0 : 1.0;

	return Round((Mean - ((double)p + 1.0) / 2.0) / ((double)n - (double)p), 1.0 / ((double)n - (double)p - Boole)) * (xmax - xmin) + xmin;
}
