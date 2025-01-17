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

#include "LoRaWANHandler.hpp"
#include <LoRaWan_APP.h>
#include <Wire.h>
#include <HT_SSD1306Wire.h>
#include <Preferences.h>

#define PREFS_NAMESPACE "appconfig"
#define PREFS_MAGIC "magic"
#define PREFS_MAGIC_VALUE 0x19660304
#define PREFS_SLEEPTIME "sleeptime"
#define PREFS_SLEEPTIME_DEFAULT_VALUE 1200000
#define PREFS_SEND_DELAY "senddelay"
#define PREFS_SEND_DELAY_DEFAULT_VALUE 0
#define PREFS_APP_EUI "appEui"
#define PREFS_DEV_EUI "devEui"
#define PREFS_APP_KEY "appKey"

extern SSD1306Wire display;

LoRaWANHandler loRaWANHandler;

// LORAWAN Settings /////////////////////////////////////////////////////////

uint8_t appEui[8];
uint8_t devEui[8];
uint8_t appKey[16];
uint8_t nwkSKey[16];
uint8_t appSKey[16];
uint32_t devAddr = 0;
uint32_t appTxDutyCycle;
uint16_t userChannelsMask[6] = {0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t loraWanClass = CLASS_A;

bool overTheAirActivation = true;
bool loraWanAdr = true;
bool isTxConfirmed = true;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;

// Downlink Handler /////////////////////////////////////////////////////////

/**
 * @brief Handles inbound LoRaWAN downlink messages, processes their content, 
 *        and applies relevant actions such as updating sleep time and send delay.
 *
 * This function prints debug information about the downlink message, including 
 * its associated RX window, payload size, and port. It also checks for a specific 
 * command and value pattern in the payload and invokes appropriate setter methods 
 * for device parameters.
 *
 * @param mcpsIndication Pointer to a McpsIndication_t structure containing 
 *        information about the received message (buffer, buffer size, port, etc.).
 */
void downLinkDataHandle(McpsIndication_t *mcpsIndication)
{
  Serial.printf("DOWN +REV DATA:%s,RXSIZE %d,PORT %d\r\n", mcpsIndication->RxSlot ? "RXWIN2" : "RXWIN1", mcpsIndication->BufferSize, mcpsIndication->Port);
  Serial.print("DOWN +REV DATA:");
  for (uint8_t i = 0; i < mcpsIndication->BufferSize; i++)
  {
    Serial.printf("%02X", mcpsIndication->Buffer[i]);
  }
  Serial.println();

  if (mcpsIndication->BufferSize == 6 && mcpsIndication->Buffer[0] == 0x5A ) // 0x5A is the magic number
  {
    uint8_t command = mcpsIndication->Buffer[1];
    uint32_t value = mcpsIndication->Buffer[2] << 24 | mcpsIndication->Buffer[3] << 16 | mcpsIndication->Buffer[4] << 8 | mcpsIndication->Buffer[5];

    Serial.printf("DOWN value(HEX): %08X\n", value);
    Serial.printf("DOWN value(DEC): %d\n\n", value);

    switch (command)
    {
    case 0x01:
      loRaWANHandler.setSleepTime(value);
      break;
    case 0x02:
      loRaWANHandler.setSendDelay(value);
      break;
    default:
      Serial.println("Error: Unknown command");
      break;
    }
  }
}

// LoRaWANHandler Class implementation //////////////////////////////////////

void LoRaWANHandler::printHex(char *label, uint8_t *buffer, int length)
{
  Serial.print(label);
  Serial.print(": ");
  for (int i = 0; i < length; i++)
  {
    Serial.printf("%02X", buffer[i]);
  }
  Serial.println();
}

void LoRaWANHandler::initConfig(bool showConfig)
{
  Preferences preferences;
  preferences.begin(PREFS_NAMESPACE, false);
  uint32_t magic = preferences.getUInt(PREFS_MAGIC, 0l);

  if (magic != PREFS_MAGIC_VALUE || reconfigure)  
  {
    magic = PREFS_MAGIC_VALUE;
    appTxDutyCycle = PREFS_SLEEPTIME_DEFAULT_VALUE;
    sendDelay = PREFS_SEND_DELAY_DEFAULT_VALUE;

    if ( reconfigure )
    {
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.clear();
      display.drawString(display.getWidth() / 2, display.getHeight() / 2, "RECONFIGURE");
      display.display();
      delay(2000);
    }

#ifdef CREATE_DEV_EUI_RANDOM
    for (int i = 0; i < 8; i++)
    {
      devEui[i] = esp_random() & 0xFF;
    }
#endif 

#ifdef CREATE_DEV_EUI_CHIPID
    uint64_t chipId = ESP.getEfuseMac();
    for (int i = 0; i < 8; i++)
    {
      devEui[i] = *(((uint8_t *)&chipId) + (7-i)) & 0xFF;
    }
#endif
    devEui[0] = (devEui[0] & ~1) | 2;

    for (int i = 0; i < 8; i++)
    {
      appEui[i] = esp_random() & 0xFF;
    }
    appEui[0] = (appEui[0] & ~1) | 2;

    for (int i = 0; i < 16; i++)
    {
      appKey[i] = esp_random() & 0xFF;
    }
    preferences.putUInt(PREFS_MAGIC, magic);
    preferences.putUInt(PREFS_SLEEPTIME, appTxDutyCycle);
    preferences.putUInt(PREFS_SEND_DELAY, sendDelay);
    preferences.putBytes(PREFS_APP_EUI, appEui, 8);
    preferences.putBytes(PREFS_DEV_EUI, devEui, 8);
    preferences.putBytes(PREFS_APP_KEY, appKey, 16);
  }
  else
  {
    appTxDutyCycle = preferences.getUInt(PREFS_SLEEPTIME, PREFS_SLEEPTIME_DEFAULT_VALUE);
    sendDelay = preferences.getUInt(PREFS_SEND_DELAY, PREFS_SEND_DELAY_DEFAULT_VALUE);
    preferences.getBytes(PREFS_APP_EUI, appEui, 8);
    preferences.getBytes(PREFS_DEV_EUI, devEui, 8);
    preferences.getBytes(PREFS_APP_KEY, appKey, 16);
  }
  preferences.end();

  if (showConfig)
  {
    Serial.println("AppConfig loaded.");
    printf("\nMagic: %08x\n", magic);
    printf("sleep time: %dms\n\n", appTxDutyCycle);
    printf("send delay: %dms\n\n", sendDelay);
    printHex((char *)"AppEUI/JoinEUI", appEui, 8);
    printHex((char *)"        DevEUI", devEui, 8);
    printHex((char *)"        AppKey", appKey, 16);
    Serial.println();
  }
}

void LoRaWANHandler::setSleepTime(uint32_t _sleepTime)
{
  Preferences preferences;
  preferences.begin(PREFS_NAMESPACE, false);
  preferences.putUInt(PREFS_SLEEPTIME, _sleepTime);
  preferences.end();
  appTxDutyCycle = _sleepTime;
}

void LoRaWANHandler::setSendDelay(uint32_t _sendDelay)
{
  Preferences preferences;
  preferences.begin(PREFS_NAMESPACE, false);
  preferences.putUInt(PREFS_SEND_DELAY, _sendDelay);
  preferences.end();
  sendDelay = _sendDelay;
}

void LoRaWANHandler::setup()
{
  pinMode(GPIO_NUM_0, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(Vext, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(Vext, LOW);
  reconfigure = false;
  Serial.begin(115200);

  resetReason = esp_reset_reason();
  if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_EXT)
  {
    LoRaWAN.displayMcuInit();
    delay(5000);
    Serial.println("\n\nLoRaWAN_APP " __DATE__ " " __TIME__);
    Serial.printf("  HELTEC_BOARD=%d\n", HELTEC_BOARD);
    if ( digitalRead(GPIO_NUM_0) == LOW )
    {
      reconfigure = true;
    }
    initConfig(true);
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    initConfig(false);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
  }

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
}

void LoRaWANHandler::loop()
{
  switch (deviceState)
  {
  case DEVICE_STATE_INIT:
  {
    LoRaWAN.init(loraWanClass, loraWanRegion);
    LoRaWAN.setDefaultDR(3);
    break;
  }
  case DEVICE_STATE_JOIN:
  {
    LoRaWAN.displayJoining();
    LoRaWAN.join();
    break;
  }
  case DEVICE_STATE_SEND:
  {
    prepareTxFrame(appPort);
    if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_EXT)
    {
      LoRaWAN.displaySending();
    }
    LoRaWAN.send();
    deviceState = DEVICE_STATE_CYCLE;
    break;
  }
  case DEVICE_STATE_CYCLE:
  {
    txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
    LoRaWAN.cycle(txDutyCycleTime);
    if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_EXT)
    {
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.clear();
      display.drawString(display.getWidth() / 2, display.getHeight() / 2, "DEEP SLEEP");
      display.display();
    }
    deviceState = DEVICE_STATE_SLEEP;
    break;
  }
  case DEVICE_STATE_SLEEP:
  {
    LoRaWAN.sleep(loraWanClass);
    break;
  }
  default:
  {
    deviceState = DEVICE_STATE_INIT;
    break;
  }
  }
}

uint32_t LoRaWANHandler::getSleepTime()
{
  return appTxDutyCycle;
}

uint32_t LoRaWANHandler::getSendDelay()
{
  return sendDelay;
}
