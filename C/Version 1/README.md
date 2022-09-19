# Triadic Memory C Implementation (Version 1)


#### triadicmemory.c and triadicmemory.h

Reference implementations of the Dyadic/Triadic Memory algorithms and various SDR utilities.
Can be compiled as a library. Based on 8-bit memory counters. 

The [current version](https://github.com/PeterOvermann/TriadicMemory/tree/main/C) uses binary memory locations,
reducing memory consumption to 1/8 compared to version 1.

#### triadicmemoryCL.c

Triadic Memory command line tool. Depends on triadicmemory.c and triadicmemory.h.

#### dyadicmemoryCL.c

Dyadic Memory command line tool. Depends on triadicmemory.c and triadicmemory.h.

#### temporalmemory.c

Elementary Temporal Memory algorithm and command line tool wrapper. Depends on triadicmemory.c and triadicmemory.h.

#### deeptemporalmemory.c

Deep Temporal Memory algorithm and command line tool wrapper. Depends on triadicmemory.c and triadicmemory.h.
