function decodeUplink(input) {
  var data = {};
  
  data.preamble = input.bytes[0];
  data.status = input.bytes[1];
  data.batteryVoltage = (200.0 + input.bytes[2]) / 100.0;
  data.resetReason = input.bytes[3];
  data.bootCounter = ((input.bytes[7]<<24)+(input.bytes[6]<<16)+(input.bytes[5]<<8)+input.bytes[4]);
  data.lastResetReason = input.bytes[8];
  data.crc8le = input.bytes[9];

  // 100% battery is 4.1V
  // 0% battery is 2.5V
  var battery = data.batteryVoltage;
  if ( battery > 4.1 ) {
    battery = 4.1;
  }
  data.batteryPercentage =  Math.round((battery - 2.5) / (4.1 - 2.5) * 100.0);
  if ( data.batteryPercentage < 0 )
  {
    data.batteryPercentage = 0;
  }

  return {
    data: data,
    warnings: [],
    errors: []
  };
}
