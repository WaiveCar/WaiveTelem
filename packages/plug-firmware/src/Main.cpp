#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Command.h"
#include "Config.h"
#include "Gps.h"
#include "Internet.h"
#include "JsonBuilder.h"
#include "Logger.h"
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
  int cfgInit = Config.begin();
  int loggerInit = Logger.begin();
  int cmdInit = Command.begin();
  int sysInit = System.begin();
  int mqttInit = Mqtt.begin();
  Mqtt.poll();
  int bleInit = Bluetooth.begin();
  int gpsInit = Gps.begin();
  int canInit = Can.begin();

  String sysJson = "";
  json(sysJson, "firmware", FIRMWARE_VERSION,
       "i|configFreeMem", Config.getConfigFreeMem(),
       "i|eccInit", eccInit,
       "i|sdInit", sdInit,
       "i|cfgInit", cfgInit,
       "i|loggerInit", loggerInit,
       "i|cmdInit", cmdInit,
       "i|sysInit", sysInit,
       "i|mqttInit", mqttInit,
       "i|bleInit", bleInit,
       "i|gpsInit", gpsInit,
       "i|canInit", canInit);
  System.sendInfo(sysJson);
}

void loop() {
  Watchdog.reset();
  if (!System.stayAwake()) {
    System.sleep(1);
  }
  System.keepTime();
  Mqtt.poll();
  System.poll();
  Bluetooth.poll();
  Can.poll();
}
