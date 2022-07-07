# Triadic Memory and related algorithms

Implementations of [Triadic Memory](https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf) and related algorithms in the following programming languages:

- [C](C)
- [Chez Scheme](ChezScheme)
- [Julia](Julia)
- [Mathematica](Mathematica)
- [Python](Python)


## Dyadic Memory

Dyadic Memory realizes an associative memory for sparse hypervectors which has the functionality
of a [Sparse Distributed Memory](https://en.wikipedia.org/wiki/Sparse_distributed_memory) (SDM) as proposed by [Pentti Kanerva](https://en.wikipedia.org/wiki/Pentti_Kanerva) in 1988.

The present, highly efficient algorithm was discovered in 2021 and is based on a neural network with combinatorial connectivity.

The memory stores and retrieves heteroassociations `x -> y` of sparse binary hypervectors `x` and `y`.
Sparse binary hypervectors are also known as Sparse Distributed Representations (SDR).

Here `x` and `y` are binary vectors of dimensions `n1` and `n2` and sparse populations `p1` and `p2`, respectively. 
While in typical SDM usage scenarios `n1` and `n2` are equal, the present algorithm also allows asymmetric configurations.

The capacity of a symmetric memory with dimension `n` and sparse population `p` is approximately `(p/n)^3 / 2`.
For typical values `n = 1000` and `p = 10`, about 500,000 associations can be stored and perfectly recalled.

The Dyadic Memory algorithm was initially developed in [Mathematica](Mathematica/dyadicmemory.m) language and consists of just 10 lines of code. 

The [plain C](https://github.com/PeterOvermann/TriadicMemory/blob/main/C/dyadicmemory.c) implementation best illustrates the algorithm in procedural code. This version works with vector dimensions up to 1,200.

A [memory-optimized implementation](https://github.com/PeterOvermann/TriadicMemory/blob/main/C/sparseassociativememory.c) supports hypervector dimensions up to 20,000. It can be used as a command line tool or as C library. No other SDM currently works with dimensions that large. 

A Numba-accelerated Python version is available [here](https://github.com/PeterOvermann/TriadicMemory/blob/main/Python/sdrsdm.py).

## Triadic Memory

Triadic Memory, an algorithm developed in 2021, is an associative memory that stores ordered **triples** of sparse binary hypervectors (also called SDRs).

After storing a triple {x,y,z} in memory, any of the three items can be recalled by specifying the other two parts: `{_,y,z}` recalls `x`, `{x,_,z}` recalls `y`, and `{x,y,_}` recalls `z`. Given three items `{x,y,z}`, one can test if their association is stored in memory by calculating, for instance, the Hamming distance or overlap between `{x,y,_}` and `z`. This remarkable property, absent in hetero-associative memories, makes Triadic Memory suitable for self-supervised applications in machine learning.

The capacity of a Triadic Memory storing hypervectors of dimension `n` and sparse population `p` is `(p/n)^3`. At a typical sparsity of 1 percent, it can therefore store and perfectly retrieve one million random associations.

The original Mathematica code can be found [here](https://github.com/PeterOvermann/TriadicMemory/blob/main/Mathematica/triadicmemory.m). The [plain C](https://github.com/PeterOvermann/TriadicMemory/blob/main/C/triadicmemory.c) implementation can be used as a command line program or deployed as a library. It's also a good starting point for people wanting to port the algorithm to another programming language.

Performance-optimized implementations are available for [Python](https://github.com/PeterOvermann/TriadicMemory/blob/main/Python/sdrsdm.py), the [Julia](https://github.com/PeterOvermann/TriadicMemory/blob/main/Julia/triadicmemory.jl) language and [Chez Scheme](https://github.com/PeterOvermann/TriadicMemory/blob/main/ChezScheme/triadicmemory.ss). 



