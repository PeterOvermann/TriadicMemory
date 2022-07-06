
(*

triadicmemory.m

Mathematica reference implementation of the Triadic Memory algorithm published in
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


TriadicMemory[f_Symbol, {n_Integer, p_Integer} ] := 

	Module[  {W, tup},
				
		(* random sparse vector with dimension n and sparse population p *)
		f[] := SparseArray[RandomSample[ Range[n], p] -> Table[1, p], {n}];

		(* initialize memory *)
		W = Table[ 0, n, n, n];
   
		(* memory addresses activated by input vectors *)
		tup[x__SparseArray] := Tuples[ Flatten[ #["NonzeroPositions"] ] & /@ {x}];
   
		(* binarize a vector, using sparsity target p *)
		f[0] = SparseArray[{0}, {n}];
  	  	f[ x_] := Module[ {t = Max[1, RankedMax[x, p]]}, SparseArray[Boole[# >= t] & /@ x]]; 
   
		(* store {x,y,z} *)
		f[x_SparseArray, y_SparseArray, z_SparseArray] := (++W[[##]] &  @@@ tup[x, y, z];);
   
		(* recall x, y, or z *)
   		f[_, y_SparseArray, z_SparseArray] := f[ Plus @@ ( W[[All, #1, #2]] & @@@ tup[y, z])];
		f[x_SparseArray, _, z_SparseArray] := f[ Plus @@ ( W[[#1, All, #2]] & @@@ tup[x, z])];
		f[x_SparseArray, y_SparseArray, _] := f[ Plus @@ ( W[[#1, #2, All]] & @@@ tup[x, y])];
   
   		];


