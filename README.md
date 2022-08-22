# Triadic Memory and related algorithms

This repository is a collection of associative memory algorithms for sparse distributed representations, which is our brain's data structure.

The Triadic Memory algorithm was discovered in 2021 and first published [here](https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf).
Subsequently, a variety of related algorithms have been derived from Triadic Memory.

Models of brain functions can be designed and implemented by creating circuits from the algorithmic components in this repository. An example
is the Deep Temporal Memory algorithm, a recurring neural network based on multiple triadic memory instances and feedback lines.

The goal of this project is to build circuits able to solve increasingly complex AI tasks with very simple programs that are rooted in the brain's core cognitive algorithms.

Various examples can be found [here](Examples).


## Implementations 

Implementations of Triadic Memory and related algorithms are available in a growing number of programming languages:

- [C](C)
- [Chez Scheme](ChezScheme)
- Java (looking for contributors)
- [Javascript](Javascript)
- [Julia](Julia)
- [Mathematica](Mathematica), with executable [examples](Mathematica/Notebooks)
- [Odin](Odin)
- [Python](Python)
- Scala (looking for contributors)



## Triadic Memory

Triadic Memory is an associative memory that stores ordered **triples** of sparse binary hypervectors (also called sparse distributed representations, or SDRs).

As a content-addressable triple store, Triadic Memory is naturally suited for storing [semantic information](Examples/Triadic%20Memory%20-%20Semantic%20Triples%20and%20Analogies.pdf).

Triadic memory allows tri-directional queries: Any part of an SDR triple can be recalled from the other two parts.One can test if a specific association is already stored in memory. This remarkable property, absent in hetero-associative memories, makes Triadic Memory suitable for self-supervised machine learning tasks.

Triadic Memory learns new information in one shot. Stored data can be recalled from incomplete or noisy input data.

A Triadic Memory has the capacity to store `(n/p)^3` random triples of hypervectors with dimension `n` and sparse population `p`. At a typical sparsity of 1 percent, it can therefore store and perfectly retrieve one million triples.

The original Mathematica code can be found [here](Mathematica/triadicmemory.m). The [plain C](C/triadicmemory.c) implementation can be compiled as a command line program or as a library. It's also a good starting point for people wanting to port the algorithm to another programming language.

Performance-optimized implementations are available for [Python](Python/sdrsdm.py), the [Julia](Julia/triadicmemory.jl) language, [Chez Scheme](ChezScheme/triadicmemory.ss), 
[Javascript](Javascript/TriadicMemory.js) and [Odin](Odin/triadic/triadic_memory.odin). 



## Dyadic Memory

Dyadic Memory realizes an associative memory for sparse hypervectors which has the functionality
of a [Sparse Distributed Memory](https://en.wikipedia.org/wiki/Sparse_distributed_memory) (SDM) as proposed by [Pentti Kanerva](https://en.wikipedia.org/wiki/Pentti_Kanerva) in 1988.

The present, highly efficient algorithm was discovered in 2021 and is based on a neural network with combinatorial connectivity.

The memory stores and retrieves heteroassociations `x -> y` of sparse binary hypervectors `x` and `y`.
Sparse binary hypervectors are also known as Sparse Distributed Representations (SDR).

Here `x` and `y` are binary vectors of dimensions `n1` and `n2` and sparse populations `p1` and `p2`, respectively. 
While in typical SDM usage scenarios `n1` and `n2` are equal, the present algorithm also allows asymmetric configurations.

The capacity of a symmetric memory with dimension `n` and sparse population `p` is approximately `(n/p)^3 / 2`.
For typical values `n = 1000` and `p = 10`, about 500,000 associations can be stored and perfectly recalled.

The Dyadic Memory algorithm was initially developed in [Mathematica](Mathematica/dyadicmemory.m) language and consists of just 10 lines of code. 

The [plain C](C/triadicmemory.c) implementation best illustrates the algorithm in procedural language. This version works with vector dimensions up to `n = 1,200` and can be used in an asymmetric configuration where the two sides of an association have different dimension.

A [memory-optimized implementation](C/sparseassociativememory.c) supports hypervector dimensions up to `n = 20,000`. It can be used as a command line tool or as C library. No other SDM currently works with dimensions that large. 

An Odin implementation is available [here](Odin/triadic/triadic_memory.odin) and a Numba-accelerated Python version [here](Python/sdrsdm.py).



## Deep Temporal Memory

A temporal memory processes a stream of SDRs, at each step making a prediction for the following step based on previously seen information.
It can also be used for learning separate terminated sequences.

Temporal Memory algorithms are based on circuits of two or more Triadic Memory instances with at least one feedback loop, resembling the architecture of recurrent neural networks. 

The [elementary Temporal Memory](Examples/Temporal%20Memory%20Elementary%20Algorithm.pdf) uses two Triadic Memory units arranged in the form of an Elman network.

The [Deep Temporal Memory algorithm](Examples/Deep%20Temporal%20Memory%20-%20Introduction.pdf) is a circuit of hierarchically arranged triadic memory units with multiple feedback loops. It can recognize longer and more complex temporal patterns than the elementary version based on just two memory units. 

Trained with a dataset from the [SPMF](Examples/Deep%20Temporal%20Memory%20-%20SPMF%20Sequence%20Example.pdf) project, Deep Temporal Memory achieves a prediction accuracy of 99.5 percent.

A plain C implementation can be found [here](C/deeptemporalmemory.c).

