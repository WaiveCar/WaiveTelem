#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <JsonLogger.h>
#include <WDTZero.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Command.h"
#include "Config.h"
#include "Eeprom.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Motion.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

void shutdown() {
  logDebug(NULL);
  // uint32_t top = 0;
  // for (int i = 0; (uint32_t)(&top + i) < 0x20008000; i++) {
  //   uint32_t value = *(&top + i);
  //   if (value >= 0x6000 && value < 0x20000 && value & 0x1) {
  //     Serial.print((uint32_t)(&top + i), HEX);
  //     Serial.print(String(" ") + String(i) + " ");
  //     Serial.println(value, HEX);
  //   }
  // }
}

void setup() {
  Serial.begin(115200);
#ifdef DEBUG
  delay(5000);  // to see beginning of the login
#endif
  Watchdog.attachShutdown(shutdown);
  Watchdog.setup(WDT_SOFTCYCLE16S);

  Pins.begin();
  int eccInit = ECCX08.begin();
  int sdInit = SD.begin(SD_CS_PIN);
  int loggerInit = Logger.begin();  // dependent on SD.begin()
  int cfgInit = Config.begin();     // dependent on SD.begin()
  int eepromInit = Eeprom.begin();  // dependent on ECCX08.begin() and SD.begin()
  System.begin();                   // dependent on ECCX08.begin()
  Mqtt.begin();                     // dependent on System.begin()
  Mqtt.poll();
  int motionInit = Motion.begin();
  Gps.begin();

  char initStatus[128], sysJson[256];
  json(initStatus, "-{",
#ifndef ARDUINO_SAMD_MKR1000
       "modem", Internet.getModemVersion(),  // dependent on Mqtt.poll()
#endif
       "i|ecc", eccInit,
       "i|sd", sdInit,
       "i|eeprom", eepromInit,
       "i|motion", motionInit,
       "i|cfg", cfgInit,
       "i|logger", loggerInit);
  json(sysJson, "firmware", FIRMWARE_VERSION, "i|configFreeMem", Config.getConfigFreeMem(), initStatus);
  System.sendInfo(sysJson);
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
