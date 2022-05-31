
from sdrsdm import TriadicMemory, randomSDR
from time import time
import numpy as np


test_len = 1000000        ############################################## How many SDRs will be written to memory 

SDR_SIZE = 1000           ############################################# The size of the test SDRs
SDR_BITS =   10           ############################################# How many ON bits


tm = TriadicMemory(SDR_SIZE, SDR_BITS)  ############################### Initialise the TriadicMemory

print(f"\n\nTesting TriadicMemory with {test_len} SDRs of size {SDR_SIZE} and solidity of {SDR_BITS} ON bits\n")

t = time()
indata = randomSDR(test_len + 2, N = SDR_SIZE, P = SDR_BITS)
t = int((time() - t) * 1000)
print(f"{test_len} vectors generated in {t} ms")

t = time()
for k in range(test_len):
    tm.store(indata[k], indata[k+1], indata[k+2])  ################### That's how triples are stored
t = int((time() - t)*1000)

print(f"{test_len} triples stored in {t} ms")

print("Begin retrieval test...") 

t = time() 
count_errs = {} 
value_errs = {} 
resp_list = []
for k in range(test_len): 
    answer = tm.query(indata[k], indata[k+1], None)  ############### That's how to query the "z" part of the triple
    resp_list.append(answer)
t = int((time()-t)*1000) 
print(f"{test_len} queries in {t} ms")

print("searching for errors...")
for k in range(test_len):
    resp = resp_list[k]
    if len(resp) != tm.P:
        count_errs[k] = (resp, indata[k+2]) 
    elif (indata[k+2] == resp).sum() < tm.P:
        value_errs[k] = (resp, indata[k+2]) 


print(f"{len(count_errs)} size errors - see count_errs variable") 
print(f"{len(value_errs)} value errors - see value_errs variable")

