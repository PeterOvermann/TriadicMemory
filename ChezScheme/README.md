### Triadic Memory in Scheme

[Chez Scheme](https://github.com/cisco/ChezScheme/blob/main/README.md) implementation of [Triadic Memory](https://github.com/PeterOvermann/Writings/blob/main/TriadicMemory.pdf).

Translated from an [earlier version](https://github.com/rogerturner/TriadicMemory/blob/main/ChezScheme/original%20triadicmemory.c) of the C implementation.

triadicmemory.ss is a [R<sup>6</sup>RS Scheme](http://www.r6rs.org) library: it should be usable with any R<sup>6</sup>RS conformant Scheme system.

triadic-test.ss is a Chez Scheme benchmark/test script which compiles and executes itself using parameters which optimize for execution speed:
```
ChezScheme % scheme
Chez Scheme Version 9.5.7.6
Copyright 1984-2021 Cisco Systems, Inc.

> 
ChezScheme % triadic-test.ss z 1000 10
Testing TriadicMemory with 100000 SDRs of size 1000 and solidity of 10 ON bits
100000 vectors generated in 158ms
100000 triples stored in 1248ms
100000 queries in 6327ms
0 errors
ChezScheme % triadic-test.ss x
Testing TriadicMemory with 100000 SDRs of size 1000 and solidity of 10 ON bits
100000 vectors generated in 157ms
100000 triples stored in 1253ms
100000 queries in 8712ms
0 errors
ChezScheme % triadic-test.ss y
Testing TriadicMemory with 100000 SDRs of size 1000 and solidity of 10 ON bits
100000 vectors generated in 162ms
100000 triples stored in 1252ms
100000 queries in 23837ms
0 errors
```

#### Installing Chez Scheme

A Windows .exe is [available](https://github.com/cisco/ChezScheme/releases)

For other platforms (including Apple Silicon) the [Racket fork](https://github.com/racket/racket/tree/master/racket/src/ChezScheme) with [these](https://racket.discourse.group/t/running-chez-after-building-racket/329) [instructions](https://github.com/racket/racket/blob/master/racket/src/ChezScheme/BUILDING) is suggested.
