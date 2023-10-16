import argparse

from sdrsdm import TriadicMemory
from mem_sdrsdm import TriadicMemory as MemTriadicMemory
from sdr_util import random_sdrs, sdr_distance
from time import time
import numpy as np


SDR_SIZE = 1000           ################################# The size of the test SDRs
SDR_BITS =   10           ################################# How many ON bits

def test_triadic(mem, test_len):
    print(f"\n\nTesting TriadicMemory with {test_len} SDRs of size {SDR_SIZE} and solidity of {SDR_BITS} ON bits\n")

    t = time()
    X = random_sdrs(test_len + 2, SDR_SIZE, SDR_BITS)
    t = int((time() - t) * 1000)
    print(f"{test_len} vectors generated in {t} ms")

    t = time()
    for k in range(test_len):
        mem.store(X[k], X[k+1], X[k+2])  ################### That's how triples are stored
    t = int((time() - t)*1000)

    print(f"{test_len} triples stored in {t} ms")

    print("Begin retrieval test...") 

    t = time() 
    resp_list = []
    for k in range(test_len): 
        answer = mem.query(X[k], X[k+1], None)  ############### That's how to query the "x" part of the triple
        resp_list.append(answer)
    t = int((time()-t)*1000) 
    print(f"{test_len} queries for Z in {t} ms")

    diferents = []
    print("searching for errors...")
    for k in range(test_len):
        dist = sdr_distance(resp_list[k], X[k+2])
        if dist > 0.0: 
            diferents.append((k, dist))


    print(f"{len(diferents)} errors - see diferents variable") 

    ################### X queries... 20x slower than querying for Z
    print("Now query for X-es...")
    t = time()
    x_resp_list = []
    for k in range(test_len):
        answer = mem.query(None, X[k+1], X[k+2]) 
        x_resp_list.append(answer)
    t = int((time() - t)*1000)
    print(f"{test_len} queries for X in {t} ms")

    x_diferents = []
    for k in range(test_len):
        dist = sdr_distance(x_resp_list[k], X[k])
        if dist > 0.0:
            x_diferents.append((k, dist))
    print(f"{len(x_diferents)} errors - see x_diferents variable") 

    #################  Y queries...
    print("Now query for Y-es...")
    t = time()
    y_resp_list = []
    for k in range(test_len):
        answer = mem.query(X[k], None, X[k+2]) 
        y_resp_list.append(answer)
    t = int((time() - t)*1000)
    print(f"{test_len} queries for Y in {t} ms")

    y_diferents = []
    for k in range(test_len):
        dist = sdr_distance(y_resp_list[k], X[k+1])
        if dist > 0.0:
            y_diferents.append((k, dist))
    print(f"{len(y_diferents)} errors - see y_diferents variable") 


parser = argparse.ArgumentParser(
                    prog='diadic tester',
                    description='test the diadic memory')
parser.add_argument('-c', '--count', type=int, default=100000)
parser.add_argument('-m', '--mem', action='store_true')
args = parser.parse_args()

if args.mem:
    mem = MemTriadicMemory(SDR_SIZE, SDR_BITS)
else:
    mem = TriadicMemory(SDR_SIZE, SDR_BITS)

test_triadic(mem, args.count)