# ESP32-HELTEC-LoRa-TTN-OTAA

Sample LoRaWAN code for *HELTEC WiFi LoRa V2* and *V3* boards using PlatformIO.

## TTN config data

After power on or hard reset (not after recovering from deep sleep) the board prints out on the serial line: 

```
LoRaWAN_APP Jan 15 2025 09:13:01
  HELTEC_BOARD=30
AppConfig loaded.

Magic: 19660304
Sleeptime: 60000ms

AppEUI: 0102030405060708
DevEUI: 0102030405060708
AppKey: 01020304050607080102030405060708
```

AppEUI, DevEUI, and AppKey will be randomly generated and permanently stored in the NVS Memory. Under normal circumstances, they will never change.

