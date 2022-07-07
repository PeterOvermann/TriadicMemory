# Triadic Memory

Implementations of [Triadic Memory](https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf) and related algorithms in various programming languages:

- [C](C)
- [Chez Scheme](ChezScheme)
- [Julia](Julia)
- [Mathematica](Mathematica)
- [Python](Python)


## Dyadic Memory

Dyadic Memory realizes an associative memory for sparse hypervectors which has the functionality
of a [Sparse Distributed Memory](https://en.wikipedia.org/wiki/Sparse_distributed_memory) (SDM) as proposed by Pentti Kanerva in 1988.

The present, highly efficient algorithm, discovered in 2021, is based on a neural network with combinatorial connectivity.

The program stores and retrieves heteroassociations x -> y of sparse binary hypervectors x and y.
Sparse binary hypervectors are also known as Sparse Distributed Representations (SDR).

Here x and y are binary vectors of dimensions n1 and n2 and sparse populations p1 and p2, respectively. 
While in typical SDM usage scenarios n1 and n2 are equal, the present algorithm also allows asymmetric configurations.

The capacity of a symmetric memory with dimension n and sparse population p is approximately (p/n)^3 / 2.
For typical values n = 1000 and p = 10, about 500,000 associations can be stored and perfectly recalled.

The Dyadic Memory algorithm was initially developed in [Mathematica](Mathematica/dyadicmemory.m) and consists of only 10 lines of code. 

The [plain C](https://github.com/PeterOvermann/TriadicMemory/blob/main/C/dyadicmemory.c) implementation best illustrates the algorithm in procedural language. This version works with vector dimensions up to 1,200.

This [memory-optimized implementation](https://github.com/PeterOvermann/TriadicMemory/blob/main/C/sparseassociativememory.c) supports hypervector dimensions up to 20,000. It can be used as a command line tool or C library. No other SDM currently works with dimensions of this size. 

A Numba-accelerated Python version is available [here](https://github.com/PeterOvermann/TriadicMemory/blob/main/Python/sdrsdm.py).
