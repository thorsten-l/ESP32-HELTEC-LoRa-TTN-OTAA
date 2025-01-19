function decodeUplink(input) {
  var data = {};
  
  data.preamble = input.bytes[0];
  data.status = input.bytes[1];
  data.batteryVoltage = ( 200.0 + input.bytes[2] ) / 100.0;
  data.crc8le = input.bytes[3];

  // 100% battery is 4.1V
  // 0% battery is 3.0V
  var battery = data.batteryVoltage;
  if ( battery > 4.1 ) {
    battery = 4.1;
  }
  data.batteryPercentage = Math.round((battery - 3.0) / (4.1 - 3.0) * 100.0);

  return {
    data: data,
    warnings: [],
    errors: []
  };
}
