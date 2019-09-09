#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <JsonLogger.h>
#include <WDTZero.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Config.h"
#include "Eeprom.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Motion.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

#define CRASH_REPORT_START_BYTE 52
#define CRASH_REPORT_START_DATA 0xa5

void shutdown() {
  uint32_t top;
  uint8_t data[72];
  int ret = ECCX08.readSlot(13, data, 72);
  logDebug("i|ret", ret);
  data[CRASH_REPORT_START_BYTE] = CRASH_REPORT_START_DATA;
  int j = CRASH_REPORT_START_BYTE + 2;
  for (int i = 1; j < 70 && (uint32_t)(&top + i) < 0x20008000; i++) {
    uint32_t value = *(&top + i);
    if (value >= 0x6000 && value < 0x20000 && value & 0x1) {
      value &= 0xfffffe;
      data[j] = value;
      data[j + 1] = value >> 8;
      data[j + 2] = value >> 16;
      j += 3;
      char str[7];
      sprintf(str, "%lx", value);
      Serial.println(str);
    }
  }
  int num = (j - CRASH_REPORT_START_BYTE - 2) / 3;
  data[CRASH_REPORT_START_BYTE + 1] = num;
  ret = ECCX08.writeSlot(13, data, 72);
  logDebug("i|ret", ret, "i|j", j);
}

void checkCrashReport() {
  int maxLen = (72 - CRASH_REPORT_START_BYTE - 2) / 3;
  char* list[maxLen] = {NULL};
  uint8_t data[72];
  int ret = ECCX08.readSlot(13, data, 72);
  logDebug("i|ret", ret);
  uint8_t len = data[CRASH_REPORT_START_BYTE + 1];
  if (data[CRASH_REPORT_START_BYTE] == CRASH_REPORT_START_DATA && len < 7) {
    int i = 0;
    int j = CRASH_REPORT_START_BYTE + 2;
    for (; i < len; i++) {
      uint32_t value = data[j] | data[j + 1] << 8 | data[j + 2] << 16;
      j += 3;
      list[i] = (char*)malloc(7);
      sprintf(list[i], "%lx", value & 0xffffff);
    }
    char reported[256];
    // logDebug("i|i", i);
    // logDebug("list[0]", list[0]);
    json(reported, "{|system", "{|crash", "s[backtrace", i, list, "}|", "}|");
    System.report(reported);

    data[CRASH_REPORT_START_BYTE] = 0;
    ret = ECCX08.writeSlot(13, data, 72);
    logDebug("i|ret", ret);

    for (int k = 0; k < maxLen; k++) {
      free(list[k]);
    }
  }
}

void setup() {
  Serial.begin(115200);
#ifdef DEBUG
  delay(5000);  // to see beginning of the login
#endif
  Watchdog.attachShutdown(shutdown);
  Watchdog.setup(WDT_SOFTCYCLE16S);

  Pins.begin();
  ECCX08.begin();
  int sdInit = SD.begin(SD_CS_PIN);
  Logger.begin();  // dependent on SD.begin()
  // shutdown();
  int cfgInit = Config.begin();     // dependent on SD.begin()
  int eepromInit = Eeprom.begin();  // dependent on ECCX08.begin() and SD.begin()
  System.begin();                   // dependent on ECCX08.begin()
  Mqtt.begin();                     // dependent on System.begin()
  Mqtt.poll();
  Motion.begin();
  Gps.begin();

  char sysJson[256];
  json(sysJson, "firmware", FIRMWARE_VERSION,
       "i|configFreeMem", Config.getConfigFreeMem(),
#ifndef ARDUINO_SAMD_MKR1000
       "modem", Internet.getModemVersion().c_str(),  // dependent on Mqtt.poll()
#endif
       "i|sd", sdInit,
       "i|eeprom", eepromInit,
       "i|cfg", cfgInit);
  System.sendInfo(sysJson);
  checkCrashReport();
}

void loop() {
  Watchdog.clear();
  if (!System.stayResponsive()) {
    System.sleep(1);
  }
  System.keepTime();

  Bluetooth.poll();
  Can.poll();
  System.poll();
  Motion.poll();
  Mqtt.poll();
}
