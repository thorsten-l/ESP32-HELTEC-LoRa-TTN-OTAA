#pragma once
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

/**
 * @brief Size of the application data.
 */
extern uint8_t appDataSize;

/**
 * @brief Application data buffer.
 */
extern uint8_t appData[];

extern void prepareTxFrame(uint8_t port);

/**
 * @class LoRaWANHandler
 * @brief Manages LoRaWAN communication for ESP32 devices.
 *
 * The LoRaWANHandler class handles the setup and loop operations for establishing
 * LoRaWAN connections using the Over-The-Air Activation (OTAA) method. It manages
 * configuration parameters, sleep timings, and send delays to optimize communication
 * and power usage.
 */
class LoRaWANHandler
{
  private:
    uint32_t magic;
    uint32_t sendDelay;
    esp_reset_reason_t resetReason;
    bool reconfigure;

    void printHex(char *label, uint8_t *buffer, int length);
    void initConfig(bool showConfig);

  public:
    void setup();
    void loop();

    uint32_t getSleepTime();
    uint32_t getSendDelay();

    void setSleepTime(uint32_t _sleepTime);
    void setSendDelay(uint32_t _sendDelay);
};

extern LoRaWANHandler loRaWANHandler;

