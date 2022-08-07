
(*

temporalmemory.m


Mathematica reference implementation of the temporal memory algorithm published in
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

	
TemporalMemory[t_Symbol,{n_Integer,p_Integer}]:=

	Module[ { M1, M2, overlap, y1, z1, x2, y2, z2 },

	TriadicMemory[M1, {n,p} ];	(* context *)
	TriadicMemory[M2, {n,p} ];	(* predictions *)

	overlap[a_SparseArray, b_SparseArray] := Total[BitAnd[a,b]];

	(* initialize circuit state variables with zero vectors *)
	y1 = z1 = x2 = y2 = z2 = M1[0];

	t[inp_] := Module[ {x1},

		(*flush state if input is zero -needed when used as a sequence memory*)
		If[Total[inp] == 0, Return[ y1 = z1 = x2 = y2 = z2 = M1[0]]];

		(* bundle previous input with previous context *)
		x1 = BitOr[y1,z1];

		(* store new prediction if necessary *)
		If[ z2 != (y1=inp), M2[x2, y2, y1]];

		(* create new random context if necessary *)
		If[ overlap[ M1[_, y1, z1 = M1[x1, y1, _]], x1 ] < p, M1[x1, y1, z1 = M1[]]];

		z2 = M2[x2 = x1, y2 = y1, _] (* z2 is the prediction *)
		]

	];
	
		
