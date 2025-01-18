function decodeUplink(input) {
  var data = {};
  
  data.preamble = input.bytes[0];
  data.status = input.bytes[1];
  data.battery = input.bytes[2] / 100.0;
  data.battery += 2.0;
  data.crc8le = input.bytes[3];

  return {
    data: data,
    warnings: [],
    errors: []
  };
}
