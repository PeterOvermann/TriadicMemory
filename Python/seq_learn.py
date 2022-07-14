"""
This is an attempt to implement Peter's Temporal Memory idea in python

Here are two classes - 

SequencePredictor - which doesn't actually work
but I attempted to make an algorithmically meaningful predict() function

SeqPred - which basically replicates TemporalMemory.c functionality 
this works.. ish. One "flaw" I noticed was it predicts first entry in a sequence 
after the last one. Why is that... it elludes me. 
And see comments I hate storing redundant context and meaningless variable names as if they were ... cpu registers .. just hate it.  

Also a predict() that both forces down the throat of predictor 
a new "move" while making a prediction at the same time  doesnt seem right. 

Imagine an application in which an agent makes observations on environment state changes being unable of asking "what IF we do X at current position then Y at position that follows?" without option of "IF" just remembering that he actually did made move X then Y 

It should have:
- a learn() method to learn a new sequence or to continue past learn() sequence.
- a predict() to  make predictions (and back feed them if one so whish) 
  without dancing over the sand castle built with learn()

How to try it e.g.: 

    $ ipython seq_learn.py
    > roll("write any sequence the string will be split in words by separating spaces")

    ...
    (see down here the def roll() comments)

Copyright .. as sdrsdm.py


"""

import numpy as np

import numba
from sdr_util import sdr_distance, sdr_union, empty_sdr, sdr_overlap
from sdrsdm  import TriadicMemory

class SequencePredictor:
    def __init__(self, N, P, min_distance = 0.0): 
        self.Mkeys = TriadicMemory(N,P)
        self.Mpred = TriadicMemory(N,P)
        self.min_distance = min_distance
        self.context = self.new_context()

    def new_context(self): 
        # return [empty_sdr(), empty_sdr(), empty_sdr()]
        return [random_sdr(self.Mkeys.N,self.Mkeys.P) for _ in range(3)]

    def predict(self, inp):
        Mkeys, Mpred = self.Mkeys, self.Mpred 
        prev_rkey, prev_ukey, prev_inp = self.context
        pred_inp = Mpred.query_Z(prev_ukey, prev_inp) # predicted input from context
        if sdr_distance(inp, pred_inp) > self.min_distance: 
            Mpred.store(prev_ukey, prev_inp, inp)  # input different from predicted 
        ukey = sdr_union(prev_inp, prev_rkey)      # quite redundant if you ask?
        # print(f"Distance ukey - prev_ukey is {sdr_distance(ukey,prev_ukey)}")
        rkey = Mkeys.query_Z(ukey, inp)            # look for memorized random key
        found_ukey = Mkeys.query_X(rkey, inp)  # Memorised ukey retrieved from Mkeys
        if sdr_overlap(ukey, found_ukey) < Mkeys.P:
            rkey = random_sdr(Mkeys.N, Mkeys.P)
            Mkeys.store(ukey, inp, rkey)

        self.context = (rkey, ukey, inp) 
        return Mpred.query_Z(ukey, inp)
            

class SeqPred:
    def __init__(self, N, P, distance = 0.0):
        self.M1 = TriadicMemory(N,P)
        self.M2 = TriadicMemory(N,P) 
        self.context = [empty_sdr() for _ in range(5)]
        self.distance = distance

    def predict(self, inp): 
        M1,M2 = self.M1, self.M2
        x,y,u,v,c = self.context # they will be packed back in context before return 
                             # unfortunately x duplicates u and y duplicates v

        # C:92
        x = sdr_union(y,c)   # Why preserve x in context if we don't use it? 
        # x = sdr_union(v,c) # is just as good see return below

        # C:93 
        y = inp              # What ... so y is simply inp(ut)

        # C:96
        c = M2.query_Z(u,v)  # Now c is erased ... ok - 
                             #  it simply becomes predicted input
        
        # Now comparing y (new input) with c (predicted input)
        # C:98
        if sdr_distance(y,c) > self.distance: # if distance is 0 then y == c
            M2.store(u,v,y)  # more sense makes M2.store(u,v,inp) since y IS inp

        # C:102
        c = M1.query_Z(x, y) 
        # Now c is no longer previous input! is ...used as a key for M1 ??
        # C:103
        u = M1.query_X(y, c) # ... laundromatic I suppose...
        
        # C:105
        if sdr_overlap(x, u) < M1.P: 
            c = random_sdr(M1.N, M1.P) 
            M1.store(x,y,c)

        # C:111
        u = x.copy() #!!! because... ? 
        v = y.copy() # So v is simply previous input. Why not name it this way? 
        self.context = (x,y,u,v,c) # Just because it is kept this way in C
        return M2.query_Z(u,v)     # shouldn't return earlier if prediction matches?
        # Anyway it doesn't seem to work, see roll() below 
                                   
if __name__ == "__main__":

    from sdr_util import random_sdr

    N, P = 1000,20

    def find_by_name(name,pairs): 
        for n, sdr in pairs:
            if n == name:
                return sdr
        return None

    def find_by_sdr(sdr, pairs): 
        for name, s in pairs: 
            if sdr_distance(s,sdr) == 0: 
                return name
        return None
        

    pairs = []
    predictor = SequencePredictor(N,P)
    # predictor = SeqPred(N,P)

    """
    example use: 
    roll('The brown fox jumped over the lazy dog") 

    when we want to move forward N (e.g. 15) steps by feeding prediction as input: 
    roll('The brown',15)
    """
    def roll(text, future_steps = 0): 
        string = text.split()
        print(f'rolling: "{string}" :')
        print('(INPUT,PREDICTED)... ', end='')
        for name in string:
            sdr = find_by_name(name, pairs)
            if sdr is None:
                sdr = random_sdr(N,P)
                pairs.append((name, sdr))
            pred_sdr = predictor.predict(sdr)
            pred_name = find_by_sdr(pred_sdr, pairs)
            if pred_name is None: 
                pred_name = '_'
            print(f"({name}>{pred_name}),", end = '')
        
        if future_steps: print('\nAUTO FEED: ',end='') # add a newline
        while future_steps:
            sdr = find_by_name(pred_name,pairs) 
            if sdr is None: 
                break
            pred_sdr = predictor.predict(sdr)
            pred_name = find_by_sdr(pred_sdr, pairs)
            if pred_name:
                print('>' + pred_name, end = '')
            future_steps -= 1
        print(' .')



