import SDRUtils from '../SDRUtils';

test("encode/decode for basic linear SDR [0,20]", () => {
  const minValue = 0, maxValue = 20;
  const [encoder,decoder] = SDRUtils.buildLinearEncoderDecoder(minValue, maxValue, 100, 5);
  for (let i = minValue; i <= maxValue; i++) {
    const decoded = decoder(encoder(i));
    expect(i).toBeCloseTo(decoded);
  }
});

