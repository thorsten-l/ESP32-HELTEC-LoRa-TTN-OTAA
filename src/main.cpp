/*
 * Copyright 2025 Thorsten Ludewig (t.ludewig@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <Arduino.h>
#include <LoRaWANHandler.hpp>
#include <BatteryHandler.hpp>
#include <rom/crc.h>
#include <alog.h>

/*

Schedule downlink (FPort 1)

Payload Type: Bytes
  Byte 0: 0x5A (magic number) 
   - Why 0x5A? 0x5A is a nice binary number, '0101 1010', 
     and it's easy to recognize.

  Byte 1: Command 
  Byte 2-5: Value

Commands:
  0x01: Set sleep time
  0x02: Set send delay

Example:
0x5A 0x01 0x00 0x01 0xD4 0xC0 -> Set sleep time to 120000ms (2 minutes)

The values are in big-endian format and represent a 32-bit unsigned integer.
They are permanently stored in the device's non-volatile memory.

*/

typedef struct payload
{
  uint8_t preamble;
  uint8_t status;
  uint8_t voltage;
  uint8_t reset_reason;
  uint32_t bootcounter;
  uint8_t crc;
} payload_t;

static esp_reset_reason_t reset_reason;
RTC_IRAM_ATTR static uint32_t bootcounter = 0;

/**
 * @brief Prepares the transmission frame for LoRaWAN.
 *
 * This function sets up the application data to be sent over LoRaWAN.
 * If a send delay is configured, it applies the delay before preparing the frame.
 *
 * @param port The port number on which to send the data.
 */
void prepareTxFrame(uint8_t port)
{
  // e.g. warmup delay for the sensor
  if ( loRaWANHandler.getSendDelay() > 0 )
  {
    ALOG_D("Send delay: %dms", loRaWANHandler.getSendDelay());
    delay(loRaWANHandler.getSendDelay());
  }

  float voltage = batteryHandler.getBatteryVoltage();
  ALOG_D("Battery voltage: %.2fV", voltage);
  voltage -= 2;
  voltage *= 100;
  uint8_t voltageInt = (uint8_t)voltage;
  ALOG_D("Voltage Data: %d", voltageInt);

  bootcounter++;
  ALOG_D("Bootcounter: %lu", bootcounter);
  ALOG_D("Reset reason: %d", reset_reason);
  
  appDataSize = sizeof(payload);
  payload_t *payload = (payload_t *)appData;
  payload->preamble = 0xA5;
  payload->status = 0x01;
  payload->voltage = voltageInt;
  payload->reset_reason = reset_reason;
  payload->bootcounter = bootcounter;
  payload->crc = crc8_le(0, appData, appDataSize - 1);
  ALOG_D("Payload size: %d", appDataSize);
  ALOG_D("Payload prepared.");
}

/**
 * @brief Initializes the LoRaWAN handler.
 *
 * This function sets up the LoRaWAN handler by invoking the setup method
 * of the loRaWANHander object during the initialization phase.
 */
void setup()
{
  reset_reason = esp_reset_reason();
  if( reset_reason == ESP_RST_POWERON )
  {
    bootcounter = 0;
  }
  batteryHandler.setup();
  loRaWANHandler.setup();
}

/**
 * @brief Continuously handles LoRaWAN events and maintains the connection.
 *
 * This function is repeatedly called in the main loop and delegates
 * processing to the LoRaWAN handler's loop method. It ensures that
 * LoRaWAN events are processed and the connection remains active.
 */
void loop()
{
  loRaWANHandler.loop();
}
