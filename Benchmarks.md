# Benchmarks


## Triadic Memory

Measuring the number of read and write operations performed per second for a large batch of random triples {x,y,z}.
Reading z is usually more efficient than reading y or z.



| language | storage size | n     | p      | items    | write / s | read z / s | read y / s  | read x / s | computer system      |
|:--------:|:------------:|:-----:|:------:|:--------:|:---------:|:----------:|:-----------:|:----------:|:--------------------:|
| C        | 1 bit        | 1000  | 10     | 1M       |  117000   | 14500      | 12000       |  4300      | Apple M1 16GB (2020) | 


## Dyadic Memory

Read and write operations performed per second for a large batch of random associations x->y.


| language | storage size | n     | p      | items    | write / s | read    | computer system         |
|:--------:|:------------:|:-----:|:------:|:--------:|:---------:|:-------:|:-----------------------:|
| C        | 1 bit        | 1000  | 10     | 1M       |  248000   | 30700   |  Apple M1 16GB (2020)   | 



