
(*

temporalmemory3.m


Mathematica implementation of a temporal/sequence memory algorithm based on three triadic memory units.
This is an extension of the elementary temporal memory algorithm published in
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

(* Requires TriadicMemory, defined in triadicmemory.m *)

TemporalMemory[t_Symbol, {n_Integer, p_Integer}] :=
  
	Module[ {M0, M1, M2, overlap, i, y, c, u, v, prediction},
   
		TriadicMemory[M0, {n, p}];	(* encodes bigrams *)
		TriadicMemory[M1, {n, p}];	(* encodes context *)
		TriadicMemory[M2, {n, p}]; 	(* stores predictions *)
   
		overlap[a_SparseArray, b_SparseArray] := Total[BitAnd[a, b]];
   
		(* initialize state variables with null vectors *)
		i = y = c = u = v = prediction = M1[0];
   
		t[inp_] := Module[{x, j, bigram},
     
			(* flush circuit state variables if input is zero (used for sequence termination) *)
			If[ Total[inp] === 0, Return[  i = y = c = u = v = prediction = M1[0]] ];
     
			j = i;
			bigram = M0[i = inp, j, _];
     
			If [ overlap[ M0[i, _, bigram], j] < p, M0[ i, j, bigram = M0[]]];
     
			(* bundle previous input with previous context *)
			x = BitOr[y, c]; 
			y = bigram;
			
			(* store new prediction if necessary *)
			If[ prediction != i, M2[u, v, i]];
     
			(* create new random context if necessary *)
			
			If[overlap[M1[_, y, c = M1[x, y, _]], x] < p, M1[x, y, c = M1[]]];
			
			prediction = M2[u = x, v = y, _]
			]
		];

