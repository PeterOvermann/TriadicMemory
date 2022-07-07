
(*

dyadicmemory.m

Mathematica reference implementation of the Hetero-Associative Memory algorithm published in
https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf

Copyright (c) 2021 Peter Overmann

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

(*

DyadicMemory realizes an associative memory for sparse hypervectors which has the functionality
of a Sparse Distributed Memory (SDM) as proposed by Pentti Kanerva in 1988.

The present, highly efficient algorithm is based on a neural network with combinatorial connectivity.

The program stores and retrieves heteroassociations x -> y of sparse binary hypervectors x and y.

Sparse binary hypervectors are also known as Sparse Distributed Representations (SDR).

Here x and y are binary vectors of dimensions n1 and n2 and sparse populations p1 and p2, respectively. 

While in typical SDM usage scenarios n1 and n2 are equal, the present algorithm also allows asymmetric configurations.

The capacity of a symmetric memory with dimension n and sparse population p is approximately (p/n)^3 / 2.
For typical values n = 1000 and p = 10, about 500,000 associations can be stored and perfectly recalled.

Usage examples:

Create a memory instance: 			DyadicMemory[M, {1000,10}, {1000,10}]
Generate a random pair x->y:		r = M[]
Store r:							M[r]
Recall r:							M[First[r]]

Note that the sparsity of input vectors in the function calls for storing or retrieving an association
can take any value. The parameter p2 is only used to specify the target sparse population of the output
when recalling an association.

*)

   
DyadicMemory[ f_Symbol, {n1_Integer, p1_Integer}, {n2_Integer, p2_Integer}] := 

	Module[  {W, connections},
   
		(* memory initialization (using sparse representation if n2 >= 2000 *)
		W[_] =  If[ n2 < 2000, Table[0, n2], SparseArray[ _ -> 0, n2]]; 
   
		(* generate random association x->y *)
   
		f[] := SparseArray[RandomSample[ Range[n1], p1] -> Table[1, p1], {n1}] -> 
	  		SparseArray[  RandomSample[ Range[n2], p2] -> Table[1, p2], {n2}];
   
		(* connections from input to hidden layer *)

		connections[x_SparseArray] := Module[ {k = Sort[Flatten[x["NonzeroPositions"]]]},
			Flatten[ Table[ {k[[i]], k[[j]]}, {i, 1, Length[k] - 1}, {j, i + 1, Length[k]}], 1] ];
   
		(* store x->y *)

		f[x_SparseArray -> y_SparseArray] := ((W[#] += y) & /@ connections[x];);
	
		(* recall y, given an address x *)

		f[ SparseArray[ _ -> 0, n1]] = SparseArray[_ -> 0, n2]; (* special case: zero input *)
		
		f[x_SparseArray] := Module[  {v, t },
			t = Max[1, RankedMax[v = Plus @@ (W /@ connections[x]), p2]]; SparseArray[Boole[# >= t] & /@ v]
		]
  
   ];





