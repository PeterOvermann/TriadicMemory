
(*

monadicmemory.m
Auto-associative memory for sparse distributed representations.
Used as an item store, as a clean-up memory, and for clustering/pooling.
The algorithm is based on the hetero-associative memory described in 
https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf

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


MonadicMemory[f_Symbol, {n_Integer, p_Integer}] := 
	
	Module[  {D1, D2, items = 0},
   
		DyadicMemory[ D1, {n, p}];	(* two mirrored hetero-associative memories *)
		DyadicMemory[ D2, {n, p}];
   
		(* random SDR *)
		f[] := SparseArray[RandomSample[ Range[n], p] -> Table[1, p], {n}];
   
		(* store and recall x *)
		f[x_SparseArray] := Module[ {r, hidden},
     
			r  = D2[D1[D2[D1[x]]]]; (* two roundtrips *)
			
			If[ HammingDistance[x, r] < p/2 , Return[r]];
			
			items++;
			hidden = f[]; (* create random hidden SDR *)
			D1[ x -> hidden]; D2[ hidden -> x]; 
     
			x (* return input value *)
			];
   
		
		f["Items"] := items; (* only used for diagnostics *)
		];

