# TriadicMemory
Triadic Memory Algorithm:
A Julia implementation based on https://github.com/PeterOvermann 

```
julia> include("triadicmemory.jl")
inserting 10000 vectors
preparing 10000 vectors took 0.236313212 seconds: 42316.72 per sec
inserting 10000 vectors took 0.276477713 seconds: 36169.28 per sec
  0.945253 seconds (2.51 M allocations: 3.049 GiB, 13.22% gc time, 6.47% compilation time)
querying z 10000 vectors took 0.94525197 seconds: 10579.19 per sec, 0 errors
  5.759165 seconds (2.42 M allocations: 3.044 GiB, 3.37% gc time, 0.92% compilation time)
querying y 10000 vectors took 5.75916455 seconds: 1736.36 per sec, 0 errors
  0.874020 seconds (2.48 M allocations: 3.047 GiB, 15.73% gc time, 5.88% compilation time)
querying x 10000 vectors took 0.874019583 seconds: 11441.39 per sec, 0 errors
```
