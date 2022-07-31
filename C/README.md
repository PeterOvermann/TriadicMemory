# Triadic Memory C Implementation


#### triadicmemory.c and triadicmemory.h

Reference implementations of the Dyadic/Triadic Memory algorithms, and various SDR utilities.
Can be compiled as a library.

#### triadicmemoryCL.c

Triadic Memory command line tool. Depends on triadicmemory.c and triadicmemory.h.


#### dyadicmemoryCL.c

Dyadic Memory command line tool. Depends on triadicmemory.c and triadicmemory.h.


#### sparseassociativememory.c

Memory-optimized hetero-associative memory which can handle vector dimensions up to 20,000.
Functionally equivalent to a Sparse Distributed Memory (SDM).
Standalone source file.