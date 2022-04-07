
(*

temporalmemory.m

Mathematica implementation of a basic temporal memory

Dependency: triadicmemory.m




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


TemporalMemory[ t_Symbol, {n_Integer, p_Integer} ] := 
	
	Module[ {M1, M2, x, y, c, u, v},
   
		TriadicMemory[M1, {n, p}]; (* first stage: autoencoder *)
   
		TriadicMemory[M2, {n, p}]; (* second stage: prediction *)
   
		(* initialization with zero vectors *)
		x = y = c  = u = v  =  M1[0];
   
		t[inp_] := Module[ {},
     
			x = BitOr[y, c] ;   y = inp;
     
			M2[ {u, v} -> y];
     
 			{u, v, c} = M1[ x, y, Automatic]; (* v == y *)
     
 			M2[ u, v, _] (* prediction *)
     
			]
   
		];
		
		