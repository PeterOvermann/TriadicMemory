# Test for DiadicMemory
"""
The xcount variable below sets how many records would be stored & queried within memory
"""
xcount = 450000
SDR_SIZE = 1000  # Size of SDRs in bits
SDR_BITS =   10  # Default number of ON bits (aka solidity)

from time import time
import numpy as np
from sdrsdm import DiadicMemory, randomSDR

print(f"Testing DiadicMemory for {xcount} entries,\nsdr size is {SDR_SIZE} with {SDR_BITS} of ON bits")
# The number of records to write and query

t = time()
X = randomSDR(xcount+1, N = SDR_SIZE, P = SDR_BITS)
t = time() - t
print(f"{xcount+1} random SDRs generated in {int(t*1000)}ms")
    
    
mem = DiadicMemory(SDR_SIZE,SDR_BITS) ######################################## Initialize the memory

t = time()
for i,x in enumerate(X):
    if i == xcount:
        break
    y = X[i+1]
    mem.store(x,y)   ######################################################## Store operation
t = time() - t
print(f"{xcount} writes in {int(t*1000)} ms")

print("Testing queries")

size_errors = {}
found = np.zeros((xcount,mem.P),dtype = np.uint16)
t = time()
    
for i in range(xcount): 
    qresult = mem.query(X[i]) ########################################## Query operations
    if qresult.size == mem.P:
        found[i] = qresult
    else:
        found[i] = X[i+1] # 
        size_errors[i] = qresult

t = time() - t

print(f"{xcount} queries done in {int(t*1000)}ms")

print("Comparing results with expectations")
if len(size_errors): 
    print(f"{len(size_errors)} size errors, check size_errors dictionary")
diff = (X[1:] != found)

print(f"{(diff.sum(axis=1) > 0).sum()} differences check diff array")
