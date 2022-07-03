
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

TemporalMemory[ t_Symbol, {n_Integer, p_Integer} ] := 
	Module[ {M1, M2, overlap, y, c, u, v },
   
	TriadicMemory[M1, {n, p}]; (* encodes context *)
	TriadicMemory[M2, {n, p}]; (* stores predictions *)
   
	overlap[ a_SparseArray, b_SparseArray ] := Total[BitAnd[a, b]];
   
	(* initialize state variables with null vectors *)
	y = c  = u = v  =  M1[0]; 
   
	t[inp_] := Module[ {x},	
     
		(* bundle previous input with previous context *)
		x = BitOr[y, c] ;   
     
		y = inp;
     
		(* store prediction if new *)
 		If[ HammingDistance[M2[u, v, _], y] > 0 , M2[u, v, y]]; 
     
		(* recall context code and test if it's already known *)
		c = M1[x, y, _]; 
     
		If[ overlap[M1[_, y, c], x] < p ||  overlap[M1[x, _, c], y] < p, 
       		M1[x, y, c = M1[]] ]; (* create new random context *)
     
		M2[ u = x, v = y, _] (* prediction *)
	 	]
   
   ];
		
		
