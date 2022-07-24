
(*

encoders.m

Mathematica implementations of SDR encoders and decoders.

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

*)



(* encode a number in the range from xmin to xmax as an SDR with dimension n and sparse population p *)

Real2SDR[ x_, {xmin_, xmax_}, {n_Integer, p_Integer}] := 
	
	Module[ {m},

	m = Floor[(x - xmin) / (xmax - xmin) * (n - p ) ];
	
	SparseArray[ RotateRight[ PadRight[ Table[ 1, p], n], m], {n}]
	
	];
	
	
	
(* reverse of the above encoder *)

SDR2Real[ x_SparseArray, {xmin_, xmax_}, {n_Integer, p_Integer}] := 

	Module[ {a},
	
	a = Flatten[x["NonzeroPositions"]]; 

	If[ Length[a] == 0, 0,
		N[ Round[ (Mean[a] - (p + 1)/2)/(n - p ), 1/(n - p - Boole[OddQ[n - p]]) ]* (xmax - xmin) + xmin]]

	];
