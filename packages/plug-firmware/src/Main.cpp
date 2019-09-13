#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <JsonLogger.h>
#include <WDTZero.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Config.h"
#include "Crash.h"
#include "Eeprom.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
// #include "Motion.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

bool initSent = false;
int8_t sdInit;
int8_t cfgInit;
int8_t eepromInit;

void setup() {
  Serial.begin(115200);

  Watchdog.attachShutdown(shutdown);
  Watchdog.setup(WDT_SOFTCYCLE8S);

  Pins.begin();
  ECCX08.begin();
  sdInit = SD.begin(SD_CS_PIN);
  Logger.begin();               // dependent on SD.begin()
  cfgInit = Config.begin();     // dependent on SD.begin()
  eepromInit = Eeprom.begin();  // dependent on ECCX08.begin() and SD.begin()
  System.begin();               // dependent on ECCX08.begin()
  Mqtt.begin();                 // dependent on System.begin()
  Mqtt.poll();
  //  Motion.begin();
  Gps.begin();
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
  //  Motion.poll();
  Mqtt.poll();

  if (Mqtt.isConnected() && !initSent) {
    char sysJson[128];
    json(sysJson, "firmware", FIRMWARE_VERSION, "i|debug",
#ifdef DEBUG
         1,
#else
         0,
#endif
         "i|configFreeMem", Config.getConfigFreeMem(),
#ifndef ARDUINO_SAMD_MKR1000
         "modem", Internet.getModemVersion().c_str(),
#endif
         "i|sd", sdInit,
         "i|eeprom", eepromInit,
         "i|cfg", cfgInit);
    System.sendInfo(sysJson);
    checkCrashReport();
    System.resetLastHeartbeat();  // so it is send to cloud

    initSent = true;
  }
}
