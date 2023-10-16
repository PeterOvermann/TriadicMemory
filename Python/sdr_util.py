"""

Utilities for sparsely encoded SDRs.
Using numba they-re pretty fast - generators run at 200k/sec for SDRs which are 40 bit solid. 
SDR distance measurement runs at over 2M/sec for the same, 40 bit size test

Functions:
    near_sdr()    - changes a number of bits in the input SDR 
    near_sdrs()   - generates a list of neighboring SDRs 
    random_sdrs() - generates a list of random SDRs - two consecutive
                    ones have zero overlap
                
    sdr_overlap() - measures overlap in bits between two SDRs
    sdr_distance()- a metric of distance between two SDRs

    Beware both sdr_overlap and sdr_distance work on sorted SDRs

Copyright Cezar Totth 2022

Use this as you wish, without any warranties or restrictions 
"""

import numpy as np
import numba

def empty_sdr(): 
    return np.array([], dtype = np.uint32)

def linear_encoder(min_val, max_val,sdr_size, sdr_len):
    bitrange = sdr_size - sdr_len
    vrange = max_val - min_val
    def encode(value):
        firstbit = int(bitrange * (value - min_val) / vrange + 0.4999)
        sdr = [bit for bit in range(firstbit, firstbit + sdr_len)]
        return np.array(sdr, dtype = np.uint32)
    return encode

# This is a "naive" python implementation which "compiles" well in numba
@numba.njit(fastmath = True)
def sdr_overlap(n1,n2):
    """
    returns number of number overlapping bits between two input SDRs
    """
    out = 0
    i1, i2 = 0, 0
    while i1 < n1.size and i2 < n2.size:
        v1, v2 = n1[i1], n2[i2]
        if v1 == v2:
            out += 1
            i2 += 1
        elif v1 > v2:
            i2 += 1
            continue
        i1 += 1
    return out

@numba.njit(fastmath=True)
def sdr_intersection(n1, n2): 
    """
    returns bits found in both n1 and n2
    """
    out = [] 
    i1, i2 = 0, 0
    while i1 < n1.size and i2 < n2.size:
        v1, v2 = n1[i1], n2[i2]
        if v1 == v2:
            out.append(v1)
            i2 += 1
        elif v1 > v2:
            i2 += 1
            continue
        i1 += 1
    return np.array(out, dtype = np.uint32)

@numba.njit(fastmath = True)
def sdr_union(n1,n2): 
    out = []
    i1, i2 = 0, 0
    while i1 < n1.size and i2 < n2.size:
        v1, v2 = n1[i1], n2[i2]
        if v1 == v2:
            # out.append(v1)
            i2 += 1
        elif v1 > v2:
            out.append(v2)
            i2 += 1
            continue
        out.append(v1)
        i1 += 1
    return np.array(out, dtype = np.uint32)


@numba.njit(fastmath = True)
def sdr_distance(n1, n2): 
    """
    A metric distance between two SDRs, consistent for various sizes SDRs
    Returns:
    0.0 if the two SDRs are identical. 
    1.0 if the two SDRs have zero overlapping bit

    The actual formula is 1 - 2 * (number of overlapped bits)/ (total number of bits)
    """
    return 1.0 - 2.0 * sdr_overlap(n1, n2) / (len(n1) + len(n2)) 

@numba.jit(nopython=True)
def random_sdr(sdr_size, sdr_len):
    out = np.zeros(sdr_len, dtype = np.uint32)
    r = np.random.randint(0,sdr_size)
    for i in range(sdr_len):
        while r in out:
            r = np.random.randint(0,sdr_size)
        out[i] = r
    out.sort()
    return out
    
@numba.jit(nopython=True)
def near_sdr(sdr, sdr_size, switch = 3):
    """
    returns a sdr close to input sdr by switching switch bits
    can be used to generate a random distribution of overlapping sdrs

    sdr: Input SDR
    sdr_size: total number of available bits
    switch: how many bits in input SDR will be changed.
    """
    out = sdr.copy() 
    np.random.shuffle(out) 
    for i in range(switch): 
        x = np.random.randint(0,sdr_size)
        while x in out:
            x = np.random.randint(0,sdr_size)
        out[i] = x
    out.sort()
    return out

def near_sdrs(num_sdrs, sdr_size, on_bits, switch = 3):
    """
    generates a list of slowly, randomly changing SDRs 
    first SDR is generated randomly
    each following SDR is produced by randomly changing switch bits in its previous

    num_sdrs: how many SDRs to generate
    sdr_size: SDR length
    on_bits : Solidity
    switch  : how many bits to switch from previous SDR in the list
    """
    tor = [ np.random.permutation(sdr_size).astype(np.uint32)[:on_bits] ]
    tor[0].sort()
    for _ in range(num_sdrs):
        tor.append(near_sdr(tor[-1],sdr_size, switch))
    return tor

def random_sdrs(num_sdrs, sdr_size, on_bits): 
    """
    produces a list of random SDRs

    num_sdrs: how many SDRs to generate
    sdr_size: SDR length
    on_bits : Solidity
    
    """
    return near_sdrs(num_sdrs, sdr_size, on_bits, on_bits)[1:]



if __name__ == "__main__":

    from time import time
    NUM_SDRS = 100000
    SDR_SIZE =   20000
    SDR_BITS =      40

    dummy = near_sdrs(2, SDR_SIZE, SDR_BITS) # give numba time to compile

    print(f"generating {NUM_SDRS} sdrs of solidity/size: {SDR_BITS}/{SDR_SIZE}")
    print("Near  ....")
    t = time()
    nsdrs = near_sdrs(NUM_SDRS, SDR_SIZE, SDR_BITS) 
    t = time() - t
    print(f"{NUM_SDRS} sdrs generated in {int(t*1000)} ms")
    print("Random ....")
    t = time()
    rsdrs = random_sdrs(NUM_SDRS, SDR_SIZE, SDR_BITS)
    t = time() - t
    print(f"{NUM_SDRS} sdrs generated in {int(t*1000)} ms")

    ######## example for linear_encoder() ################

    encode = linear_encoder(-1,1, 100, 10)

    zero = encode(0) 
    minus_one = encode(-1)
    plus_one = encode(1) 

    print(f"zero:      {zero}")
    print(f"minus one: {minus_one}")
    print(f"plus one:  {plus_one}")

