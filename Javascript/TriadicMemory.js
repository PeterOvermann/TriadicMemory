import ndarray from "ndarray";
import ops from "ndarray-ops";

class TriadicMemory {
  constructor(n, p) {
    this.mem = ndarray((new Uint8Array(n*n*n)).fill(0), [n,n,n]);
    this.p = p;
  }

  get n() { return this.mem.shape[0]; }

  store(x, y, z)  { TriadicMemory.storeXYZ(this.mem,x,y,z);  }
  remove(x, y, z) { TriadicMemory.removeXYZ(this.mem,x,y,z); }

  query(x, y, z=null) {
    return (
      z === null ? TriadicMemory.queryZ(this.mem,this.p,x,y) : (
      x === null ? TriadicMemory.queryX(this.mem,this.p,y,z) :
                   TriadicMemory.queryY(this.mem,this.p,x,z))
    );
  }

  static storeXYZ(mem, x, y, z) {
    for (const ax of x) {
      for (const ay of y) {
        for (const az of z) {
          mem.set(ax,ay,az, mem.get(ax,ay,az)+1);
        }
      }
    }
  }

  static removeXYZ(mem, x, y, z) {
    for (const ax of x) {
      for (const ay of y) {
        for (const az of z) {
          mem.set(ax,ay,az, Math.max(0,mem.get(ax,ay,az)-1));
        }
      }
    }
  }

  static queryZ(mem, p, x, y) {
    const n = mem.shape[0];
    const sums = ndarray(new Uint32Array(n).fill(0));
    for (const ax of x) {
      for (const ay of y) {
        ops.addeq(sums, mem.pick(ax,ay,null));
      }
    }
    return TriadicMemory.sumsToSDR(sums.data, p);
  }
  static queryY(mem, p, x, z) {
    const n = mem.shape[0];
    const sums = ndarray(new Uint32Array(n).fill(0));
    for (const ax of x) {
      for (const az of z) {
        ops.addeq(sums, mem.pick(ax,null,az));
      }
    }
    return TriadicMemory.sumsToSDR(sums.data, p);
  }
  static queryX(mem, p, y, z) {
    const n = mem.shape[0];
    const sums = ndarray(new Uint32Array(n).fill(0));
    for (const ay of y) {
      for (const az of z) {
        ops.addeq(sums, mem.pick(null,ay,az));
      }
    }
    return TriadicMemory.sumsToSDR(sums.data, p);
  }

  /**
   * Convert from an array of associative counters (sum) into a sparse, binary
   * representation with a maximum number of 1s equal to p.
   * @param {Array} sums 
   * @param {Integer} p 
   * @returns {Array} The sparse representation as an array of indicies into sums
   * where the indicies i are all values where sums[i] >= the threshold defined by p.
   */
   static sumsToSDR(sums, p) {
    const ssums = [...sums];
    ssums.sort();
    const sumsLen = ssums.length;
    const threshold = Math.max(1, ssums[sumsLen-p]);
    const result = [];
    for (let i = 0; i < sumsLen; i++) {
      if (sums[i] >= threshold) { result.push(i); }
    }
    return result;
  }

}

export default TriadicMemory;
