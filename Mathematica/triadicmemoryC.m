
(*

triadicmemoryC.m

Mathematica wrapper for the triadicmemory command line tool.

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


Requires and installation of /usr/local/bin/triadicmemory. Add to enviroment path using:
SetEnvironment[ "PATH" -> Environment["PATH"] <> ":/usr/local/bin"];

*)




TriadicMemory[f_Symbol, {n_Integer, p_Integer} ] := 

	Module[ {process, tostr},
				
		(* random sparse vector with dimension n and sparse population p *)
		f[] := SparseArray[RandomSample[ Range[n], p] -> Table[1, p], {n}];

		process = StartProcess[ {"triadicmemory", ToString[n], ToString[p]}];  Pause[2];

		tostr[x_SparseArray] := StringJoin[ ToString[#] <> " " & /@ Flatten[x["NonzeroPositions"]]] ;
		tostr[_] = "_";

		(* zero vector *)
		f[0] = SparseArray[_ -> 0, {n}];
 
		(* store {x,y,z} *)
		f[x_SparseArray, y_SparseArray, z_SparseArray] := 
		   (WriteLine[ process, "{" <> tostr[x] <> "," <> tostr[y] <> "," <> tostr[z] <> "}" ];);
		
		(* recall x, y, or z *)
		f[x_, y_, z_]  := Module[ {a},
			WriteLine[ process, "{" <> tostr[x] <> "," <> tostr[y] <> "," <> tostr[z] <> "}" ]; 
			a = ToExpression /@ StringSplit[ReadLine[process]]; SparseArray[ a -> Table[1, Length[a]], {n}]];
		
 		];






