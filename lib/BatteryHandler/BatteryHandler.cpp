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
#include "BatteryHandler.hpp"

#ifdef HELTEC_WIFI_LORA_32_V3
#define ADC_Ctrl GPIO_NUM_37
#define ADC_Input GPIO_NUM_1
#define ADC_active HIGH
#define ADC_voltage_divider 0.23648373
#endif

#ifdef HELTEC_WIFI_LORA_32_V2
#define ADC_Ctrl Vext
#define ADC_Input GPIO_NUM_37
#define ADC_active LOW
#define ADC_voltage_divider 0.337318918812814
#endif

BatteryHandler batteryHandler;

void BatteryHandler::setup()
{
  pinMode(ADC_Ctrl, OUTPUT);
  pinMode(ADC_Input, ANALOG);
  analogSetClockDiv(1);
  analogSetAttenuation(ADC_2_5db);
  adcAttachPin(ADC_Input);
  digitalWrite(ADC_Ctrl, ADC_active);
}

float BatteryHandler::getBatteryVoltage()
{
  int adcValue = analogRead(ADC_Input);
  Serial.printf("ADC value: %d\n", adcValue);
  return adcValue * 1.5 / 4096.0 / ADC_voltage_divider;
  //     =value== =db== =12 bits= =resistors voltage divider=
}
