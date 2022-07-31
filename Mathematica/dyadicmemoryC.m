
(*

dyadicmemoryC.m

Mathematica wrapper for the dyadicmemory command line tool.

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


Requires an installation of /usr/local/bin/dyadicmemory. Add to environment path using:
SetEnvironment[ "PATH" -> Environment["PATH"] <> ":/usr/local/bin"];

*)




DyadicMemory[f_Symbol, {n_Integer, p_Integer} ] := 

	Module[ {process, tostr},
				
		(* random sparse vector with dimension n and sparse population p *)
		f[] := SparseArray[RandomSample[ Range[n], p] -> Table[1, p], {n}] -> 
					SparseArray[RandomSample[ Range[n], p] -> Table[1, p], {n}];

		process = StartProcess[ {"dyadicmemory", ToString[n], ToString[p]}];  Pause[1];

		tostr[x_SparseArray] := StringJoin[ ToString[#] <> " " & /@ Sort[Flatten[x["NonzeroPositions"]]]] ;

		(* zero vector *)
		f[0] = SparseArray[_ -> 0, {n}];
 
		(* store x->y *)
		f[x_SparseArray -> y_SparseArray] := 
		   (WriteLine[ process, tostr[x] <> ", " <> tostr[y]];);
		
		(* recall y, given an address x *)

		f[SparseArray[_ -> 0, n]] = f[0]; (* special case: zero input *)
		
		f[x_SparseArray]  := Module[ {a},
			WriteLine[ process, tostr[x] ]; 
			a = ToExpression /@ StringSplit[ReadLine[process]]; SparseArray[ a -> Table[1, Length[a]], {n}]];
		
 		];






