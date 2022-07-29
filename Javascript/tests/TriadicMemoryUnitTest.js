import SDRUtils from "../SDRUtils";
import TriadicMemory from "../TriadicMemory";

test("storage and removal should result in zeroed memory", () => {
  const triM = new TriadicMemory(4, 1);
  const x = [0,0,0,1], y = [0,0,1,1], z = [0,0,1,0];
  triM.store(x,y,z);
  triM.remove(x,y,z);
  const emptyTriM = new TriadicMemory(4,1);
  expect(triM.mem).toEqual(emptyTriM.mem);
});

test("basic storage and retrieval", () => {
  const minVal = -20, maxVal = 20;
  const p = 2;
  const n = 80
  const encoder = SDRUtils.buildLinearEncoder(minVal, maxVal, n, p);
  const triM = new TriadicMemory(n, p);

  const one = encoder(1), two = encoder(2), three = encoder(3);
  triM.store(one, two, three);
  expect(triM.query(one, two)).toEqual(three);
  expect(triM.query(null, two, three)).toEqual(one);
  expect(triM.query(one, null, three)).toEqual(two);

  for (let i = minVal; i <= maxVal-2; i++) {
    const a = encoder(i);
    const b = encoder(i+1);
    const c = encoder(i+2);
    triM.store(a, b, c);
    expect(triM.query(a, b)).toEqual(c);
    expect(triM.query(null, b, c)).toEqual(a);
    expect(triM.query(a, null, c)).toEqual(b);
  }
});


