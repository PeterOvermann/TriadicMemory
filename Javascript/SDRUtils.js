import ndarray from "ndarray";

const ONE_HALF_MINUS_EPSILON = 0.5-Number.MIN_VALUE;

class SDRUtils {
  static buildLinearEncoderDecoder(minValue, maxValue, sdrSize, sdrWidth) {
    return [
      SDRUtils.buildLinearEncoder(minValue, maxValue, sdrSize, sdrWidth),
      SDRUtils.buildLinearDecoder(minValue, maxValue, sdrSize, sdrWidth)
    ];
  }

  static buildLinearEncoder(minValue, maxValue, sdrSize, sdrWidth) {
    const bitRange = sdrSize-sdrWidth;
    const valueRange = maxValue - minValue;
    
    return (value) => {
      const firstBit = Math.floor(bitRange*(value-minValue)/valueRange + ONE_HALF_MINUS_EPSILON);
      return SDRUtils.range(firstBit, firstBit+sdrWidth);
    };
  }

  static buildLinearDecoder(minValue, maxValue, sdrSize, sdrWidth) {
    const bitRange = sdrSize-sdrWidth;
    const valueRange = maxValue - minValue;
    return (sdr, rounded=true) => {
      const decoded = (sdr[0]-ONE_HALF_MINUS_EPSILON)*valueRange/bitRange + minValue;
      return rounded ? Math.round(decoded) : decoded;
    }
  }

  static range(start, end, step=1) {
    return Array.from(new Array(end-start), (_,i) => i*step + start);
  }
}

export default SDRUtils;
