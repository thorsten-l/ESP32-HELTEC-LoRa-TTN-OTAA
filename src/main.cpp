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

/*

Schedule downlink (FPort 1)

Payload Type: Bytes
  Byte 0: 0x5A (magic number)
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


/**
 * @brief Prepares the transmission frame for LoRaWAN.
 *
 * This function sets up the application data to be sent over LoRaWAN.
 * If a send delay is configured, it applies the delay before preparing the frame.
 *
 * @param port The port number on which to send the data.
 */
// uint8_t appData[255]
void prepareTxFrame(uint8_t port)
{
  // e.g. warmup delay for the sensor
  if ( loRaWANHander.getSendDelay() > 0 )
  {
    delay(loRaWANHander.getSendDelay());
  }

  appDataSize = 4;
  appData[0] = 0x00;
  appData[1] = 0x01;
  appData[2] = 0x02;
  appData[3] = 0x03;
}

/**
 * @brief Initializes the LoRaWAN handler.
 *
 * This function sets up the LoRaWAN handler by invoking the setup method
 * of the loRaWANHander object during the initialization phase.
 */
void setup()
{
  loRaWANHander.setup();
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
  loRaWANHander.loop();
}
