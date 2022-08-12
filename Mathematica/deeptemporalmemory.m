
(*

deeptemporalmemory.m

Mathematica implementation of a Deep Temporal Memory based on multiple triadic memory instances
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


Requires TriadicMemory.

*)



R[f_Symbol, {n_Integer, p_Integer} ] := Module[ {T, x, y, z, overlap},

	(* instantiate a triadic memory unit *)

	TriadicMemory[ T, {n, p}];

	overlap[a_SparseArray, b_SparseArray] := Total[BitAnd[a, b]];

	x = y = z = SparseArray[{0}, {n}];

	(* reset x, y, z if input is zero *)

	f[SparseArray[{0}, {n}]] := x = y = z = SparseArray[{0}, {n}];

	f[input_SparseArray] := Module[ {},

		x = BitOr[y, z]; (* binarize x and y using ranked-max algorithm *)
		y = input;

		If[Total[x] > 0 && overlap[T[_, y, z = T[x, y, _]], x] < p, T[x, y, z = T[]]];

		z
		]
	];



TemporalMemory[t_Symbol, {n_Integer, p_Integer}] :=

	Module[{M, R1, R2, R3, R4, R5, R6, R7, x, y, z, t1, t2, t3, t4, t5, t6, t7, t8},

	(* predictions / readout memory *)
	TriadicMemory[M, {n, p}];

	(* bigram encoder units *)
	R[ #, {n, p}] & /@ {R1, R2, R3, R4, R5, R6, R7 };

	(* initialize state variables with null vectors *)
	x = y = z = t1 = t2 = t3 = t4 = t5 = t6 = t7 = M[0];

	t[inp_] := Module[ {},

		(* flush state if input is zero - needed when used as a sequence memory *)
		If[Total[inp] == 0, x = y = z = M[0]];

		(* store new prediction if necessary *)
		If[z != inp, M[x, y, inp]];

		(* encoding chain *)
		t1 = R1[inp];
		t2 = R2[t1]; 
		t3 = R3[t2];
		t4 = R4[t3];
		t5 = R5[t4];
		t6 = R6[t5];
		t7 = R7[t6];

		(* prediction readout from t1, t2, t4 and t7 *)
		z = M[x = BitOr[t1, t4], y = BitOr[t2, t7], _] 
		]

	];

