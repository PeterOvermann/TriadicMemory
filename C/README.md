# Triadic Memory C Implementation


#### triadicmemory.c and triadicmemory.h

Reference implementations of the Dyadic/Triadic Memory algorithms and various SDR utilities.
Can be compiled as a library. Uses 1-bit storage locations. The original implementation with 8-bit counters is archived [here](https://github.com/PeterOvermann/TriadicMemory/tree/main/C/Version%201).

#### triadicmemoryCL.c

Triadic Memory command line tool. Depends on triadicmemory.c and triadicmemory.h.

#### dyadicmemoryCL.c

Dyadic Memory command line tool. Depends on triadicmemory.c and triadicmemory.h.

#### temporalmemory.c

Elementary Temporal Memory algorithm and command line tool wrapper. Depends on triadicmemory.c and triadicmemory.h.

#### deeptemporalmemory.c

Deep Temporal Memory algorithm and command line tool wrapper. Depends on triadicmemory.c and triadicmemory.h.


#### triadicmemorytest.c and dyadicmemorytest.c

Performance and capacity tests. Results [here](https://github.com/PeterOvermann/TriadicMemory/blob/main/Benchmarks.md)
