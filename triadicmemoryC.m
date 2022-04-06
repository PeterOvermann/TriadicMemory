
(*

triadicmemoryC.m

Mathematica reference implementation of the Triadic Memory algorithm, using an external process for core memory functions

Requires triadicmemory command line tool to be installed. Add to enviroment path using:
SetEnvironment[ "PATH" -> Environment["PATH"] <> ":/usr/local/bin"];



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

	Module[ { process, tostr},
				
		(* random sparse vector with dimension n and sparse population p *)
		f[] := SparseArray[RandomSample[ Range[n], p] -> Table[1, {p}], {n}];


		process = StartProcess[ {"triadicmemory", ToString[n], ToString[p]}];  Pause[2];

		tostr[x_SparseArray] := StringJoin[ ToString[#] <> " " & /@ Flatten[x["NonzeroPositions"]]] ;
		tostr[_] = "_";


		(* binarize a vector, using sparsity target p *)
		f[0] = SparseArray[{0}, {n}];
  	  	f[ x_] := Module[ {t = Max[1, RankedMax[x, p]]}, SparseArray[Boole[# >= t] & /@ x]]; 
   
	
		(* store {x,y,z} *)
		f[x_SparseArray, y_SparseArray, z_SparseArray] := Module[ {}, 
		   WriteLine[ process, "{" <> tostr[x] <> "," <> tostr[y] <> "," <> tostr[z] <> "}" ]; ];
		
		(* autoencoder functions *)
 		f[ x_SparseArray, y_SparseArray, Automatic ] := Module[ {u, v, z}, 
	 		z = f[x, y, _]; u = f[_, y, z]; v = f[x, _, z]; 
	 	   	If [ HammingDistance[ x, u] + HammingDistance[y, v] > p , 
	  	  	f[x, y, z = f[]]; {x, y, z}, {u, v, z}]];
   
 		f[ x_SparseArray, Automatic, z_SparseArray ] := Module[ {u, y, w}, 
			y = f[x, _, z]; u = f[_, y, z]; w = f[x, y, _]; 
			If [ HammingDistance[ x, u] + HammingDistance[z, w] > p , 
			f[x, y = f[], z]; {x, y, z}, {u, y, w}]];
   
		f[ Automatic, y_SparseArray, z_SparseArray ] := Module[ {x, v, w}, 
			x = f[_, y, z];  v = f[x, _, z]; w = f[x, y, _]; 
			If [ HammingDistance[ y, v] + HammingDistance[z, w] > p , 
			f[x = f[], y, z ]; {x, y, z}, {x, v, w}]];
   
		(* recall x, y, or z *)
		f[x_, y_, z_]  := Module[ {a},
			WriteLine[ process, "{" <> tostr[x] <> "," <> tostr[y] <> "," <> tostr[z] <> "}" ]; 
			a = ToExpression /@ StringSplit[ReadLine[process]]; SparseArray[ a -> Table[1, {Length[a]}] , {n}]];
		
   
		(* cross-association {x,y} -> z *)
		f[ {x_SparseArray, y_SparseArray} -> z_SparseArray] := If[HammingDistance[ f[x, y, _], z] > 0, f[x, y, z]];
 	   	
		];






