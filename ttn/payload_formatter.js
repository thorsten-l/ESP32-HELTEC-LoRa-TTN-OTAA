function decodeUplink(input) {
  var data = {};
  
  data.preamble = input.bytes[0];
  data.status = input.bytes[1];
  data.battery = input.bytes[2] / 100.0;
  data.battery += 2.0;
  data.crc8le = input.bytes[3];

  // 100% battery is 4.1V
  // 0% battery is 3.0V
  var battery = data.battery;
  if ( data.battery > 4.1 ) {
    battery = 4.1;
  }
  data.batteryPercentage = (battery - 3.0) / (4.1 - 3.0) * 100.0;

  return {
    data: data,
    warnings: [],
    errors: []
  };
}
