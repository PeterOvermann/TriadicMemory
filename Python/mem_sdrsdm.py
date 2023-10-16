"""
This code is an implementation of the Triadic Memory and Dyadic Memory algorithms

Copyright (c) 2021-2022 Peter Overmann
Copyright (c) 2022 Cezar Totth
Copyright (c) 2023 Clément Michaud

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

def xaddr(x):
    addr = []
    for i in range(1,len(x)):
        for j in range(i):
            yield x[i] * (x[i] - 1) // 2 + x[j]

def store_xy(mem, x, y):
    """
    Stores Y under key X
    Y and X have to be sorted sparsely encoded SDRs
    """ 
    for addr in xaddr(x):
        for j in y:
            mem[addr][j] = 1

def store_xyz(mem, x, y, z):
    """
    Stores X, Y, Z triplet in mem
    All X, Y, Z have to be sparse encoded SDRs
    """
    for ax in x:
        for ay in y:
            for az in z:
                mem['x'][ay][az][ax] = 1
                mem['y'][ax][az][ay] = 1
                mem['z'][ax][ay][az] = 1

def query(mem, N, P, x):
    """
    Query in DiadicMemory
    """
    sums = np.zeros(N, dtype=np.uint32)
    for addr in xaddr(x):
        for k, v in mem[addr].items():
            sums[k] += v
    return sums2sdr(sums, P)

def queryZ(mem, N, P, x, y):
    sums = np.zeros(N, dtype=np.uint32)
    for ax in x:
        for ay in y:
            # print(ax, ay, mem['z'][ax][ay])
            for k, v in mem['z'][ax][ay].items():
                sums[k] += v
    return sums2sdr(sums, P)

def queryX(mem, N, P, y, z):
    sums = np.zeros(N, dtype=np.uint32)
    for ay in y:
        for az in z:
            for k, v in mem['x'][ay][az].items():
                sums[k] += v
    return sums2sdr(sums, P)

def queryY(mem, N, P, x, z):
    sums = np.zeros(N, dtype=np.uint32)
    for ax in x:
        for az in z:
            for k, v in mem['y'][ax][az].items():
                sums[k] += v
    return sums2sdr(sums,P)

@numba.jit(nopython=True)
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
        self.mem = {
            # We store the data 3 times to be able to query any of the variable but
            # if one knows which variable is to be queried, we can get rid of 2/3 of
            # the memory used.
            'x': defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: 0))),
            'y': defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: 0))),
            'z': defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: 0))),
        } 
        self.P = P
        self.N = N

    def store(self, x, y, z):
        store_xyz(self.mem, x, y, z)

    def query(self, x, y, z = None): 
        # query for either x, y or z. 
        # The queried member must be provided as None
        # the other two members have to be encoded as sorted sparse SDRs
        if z is None:
            return queryZ(self.mem, self.N, self.P, x, y)
        elif x is None:
            return queryX(self.mem, self.N, self.P, y, z)
        elif y is None:
            return queryY(self.mem, self.N, self.P, x, z)

    def query_X(self, y, z):
        return queryX(self.mem, self.N, self.P, y, z)

    def query_Y(self, x, z):
        return queryY(self.mem, self.N, self.P, x, z)

    def query_Z(self, x, y):
        return queryZ(self.mem, self.N, self.P, x, y)

    def query_x_with_P(self, y, z, P): 
        return queryX(self.mem, self.N, P, y, z)

    def size(self):
        # TODO: computing the size everytime might not be very efficient,
        # instead we could probably keep track of how many bits are stored.
        size = 0
        for x in self.mem['x'].values():
            for v1 in x.values():
                size += len(v1)
        return size * 3

from collections import defaultdict

class DiadicMemory:
    """
    this is a convenient object front end for SDM functions
    """
    def __init__(self, N, P):
        """
        N is SDR vector size, e.g. 1000
        P is the count of solid bits e.g. 10
        """
        self.mem = defaultdict(lambda: defaultdict(lambda: 0))
        self.N = N
        self.P = P

    def store(self, x, y):
        store_xy(self.mem, x, y)

    def query(self, x):
        return query(self.mem, self.N, self.P, x)

    def size(self):
        size = 0
        for v in self.mem.values():
            size += len(v)
        return size