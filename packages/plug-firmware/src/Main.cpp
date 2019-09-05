#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <JsonLogger.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Command.h"
#include "Config.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Motion.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

void setup() {
  Serial.begin(115200);
#ifdef DEBUG
  delay(5000);  // to see beginning of the login
#endif
  Watchdog.enable(WATCHDOG_TIMEOUT);

  Pins.begin();
  int eccInit = ECCX08.begin();
  int sdInit = SD.begin(SD_CS_PIN);
  int loggerInit = Logger.begin();  // dependent on SD.begin()
  int motionInit = Motion.begin();
  int cfgInit = Config.begin();  // dependent on SD.begin()

  // long data = 0;
  // //ECCX08.writeSlot(8, (byte *)&data, 4);
  // ECCX08.readSlot(8, (byte *)&data, 4);
  // logDebug("i|data", data);

  Command.begin();              // dependent on ECCX08.begin()
  System.begin();               // dependent on ECCX08.begin()
  int certInit = Mqtt.begin();  // dependent on System.begin()
  Mqtt.poll();
  Gps.begin();

  char initStatus[128], sysJson[256];
  json(initStatus, "-{",
#ifndef ARDUINO_SAMD_MKR1000
       "modem", Internet.getModemVersion(),  // dependent on Mqtt.poll()
#endif
       "i|ecc", eccInit,
       "i|sd", sdInit,
       "i|motion", motionInit,
       "i|cfg", cfgInit,
       "i|logger", loggerInit,
       "i|cert", certInit);
  json(sysJson, "firmware", FIRMWARE_VERSION, "i|configFreeMem", Config.getConfigFreeMem(), initStatus);
  System.sendInfo(sysJson);
}

void loop() {
  Watchdog.reset();
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
