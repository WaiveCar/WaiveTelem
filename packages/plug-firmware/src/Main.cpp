#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>
#ifndef ARDUINO_SAMD_MKR1000
#include <Modem.h>
#endif
#include <json_builder.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Command.h"
#include "Config.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

void setup() {
  Serial.begin(115200);
#ifdef DEBUG
  delay(4000);  // to see beginning of the login
#endif
  Watchdog.enable(WATCHDOG_TIMEOUT);

  Pins.begin();
  int eccInit = ECCX08.begin();
  int sdInit = SD.begin(SD_CS_PIN);
  int cfgInit = Config.begin();
  int loggerInit = Logger.begin();
  int cmdInit = Command.begin();
  System.begin();
  int mqttInit = Mqtt.begin();
  Mqtt.poll();
  Gps.begin();

#ifndef ARDUINO_SAMD_MKR1000
  String modemResponse = "";
  MODEM.send("ATI9");
  MODEM.waitForResponse(100, &modemResponse);
#endif

  char initStatus[128], sysJson[256];
  json(initStatus, "-{",
#ifndef ARDUINO_SAMD_MKR1000
       "modem", modemResponse.c_str(),
#endif
       "i|ecc", eccInit,
       "i|sd", sdInit,
       "i|cfg", cfgInit,
       "i|logger", loggerInit,
       "i|cmd", cmdInit,
       "i|mqtt", mqttInit);
  json(sysJson, "firmware", FIRMWARE_VERSION, "i|configFreeMem", Config.getConfigFreeMem(), initStatus);
  System.sendInfo(sysJson);
}

void loop() {
  Watchdog.reset();
  if (!System.stayResponsive()) {
    System.sleep(1);
  }
  System.keepTime();

  Mqtt.poll();
  Bluetooth.poll();
  Can.poll();
  System.poll();
}
