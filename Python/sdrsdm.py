"""
This code is an implementation of the Triadic Memory and Dyadic Memory algorithms

Copyright (c) 2021-2022 Peter Overmann
Copyright (c) 2022 Cezar Totth

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
"""
import numpy as np
import numba

@numba.jit
def xaddr(x, N):
    addr = []
    for i in range(1,len(x)):
        for j in range(i):
            addr.append(x[i]*(x[i]-1)//2 + x[j]) 
    return addr


@numba.jit
def store_xy(mem, N, x, y):
    """
    Stores Y under key X
    Y and X have to be sorted sparsely encoded SDRs
    """ 
    for addr in xaddr(x, N):
        for j in y:
            mem[addr,j] = 1

@numba.jit
def store_xyz(mem, x, y, z): 
    """
    Stores X, Y, Z triplet in mem
    All X, Y, Z have to be sparse encoded SDRs
    """
    for ax in x:
        for ay in y:
            for az in z:
                mem[ax, ay, az] = 1

    
@numba.jit
def query(mem, N, P, x):
    """
    Query in DiadicMemory
    """
    sums = np.zeros(mem.shape[1],dtype=np.uint32)
    for addr in xaddr(x, N):
        sums += mem[addr]
    return sums2sdr(sums, P)

@numba.jit
def queryZ(mem, P, x, y):
    N = mem.shape[0]
    sums = np.zeros(N, dtype = np.uint32)
    for ax in x:
        for ay in y:
            sums += mem[ax, ay, :]
    return sums2sdr(sums, P)

@numba.jit
def queryX(mem, P, y, z):
    N = mem.shape[0]
    sums = np.zeros(N, dtype = np.uint32)
    for ay in y:
        for az in z:
            sums += mem[:, ay, az]
    return sums2sdr(sums, P)

@numba.jit
def queryY(mem, P, x, z):
    N = mem.shape[0]
    sums = np.zeros(N, dtype = np.uint32)
    for ax in x:
        for az in z:
            sums += mem[ax,:,az]
    return sums2sdr(sums,P)

@numba.jit
def sums2sdr(sums, P):
    # this does what binarize() does in C
    ssums = sums.copy()
    ssums.sort()
    threshval = ssums[-P]
    if threshval == 0:
        return np.where(sums)[0]  # All non zero values
    else:
        return np.where(sums >= threshval)[0] # 

class TriadicMemory:
    def __init__(self, N, P):
        self.mem = np.zeros((N,N,N), dtype=np.uint8)
        self.P = P

    def store(self, x, y, z):
        store_xyz(self.mem, x, y, z)

    def query(self, x, y, z = None): 
        # query for either x, y or z. 
        # The queried member must be provided as None
        # the other two members have to be encoded as sorted sparse SDRs
        if z is None:
            return queryZ(self.mem, self.P, x, y)
        elif x is None:
            return queryX(self.mem, self.P, y, z)
        elif y is None:
            return queryY(self.mem, self.P, x, z)

    def query_X(self, y, z):
        return queryX(self.mem,self.P,y,z)

    def query_Y(self, x, z):
        return queryY(self.mem, self.P, x, z)

    def query_Z(self, x, y):
        return queryZ(self.mem, self.P, x,y)

    def query_x_with_P(self, y, z, P): 
        return queryX(self.mem, P, y, z)

    @property
    def N(self):
        return self.mem.shape[0]

class DiadicMemory:
    """
    this is a convenient object front end for SDM functions
    """
    def __init__(self,mem_size, N,P):
        """
        N is SDR vector size, e.g. 1000
        P is the count of solid bits e.g. 10
        """
        self.mem = np.zeros((N*(N-1)//2, N), dtype = np.uint8)
        print(f"DiadicMemory size {self.mem.size/1000000} M bytes")
        self.N = N
        self.P = P

    def store(self, x, y):
        store_xy(self.mem, self.N, x, y)

    def query(self, x):
        return query(self.mem, self.N, self.P, x)

