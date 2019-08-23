#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>
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

  char initJson[128], sysJson[256];
  json(initJson, "i|ecc", eccInit,
       "i|sd", sdInit,
       "i|cfg", cfgInit,
       "i|logger", loggerInit,
       "i|cmd", cmdInit,
       "i|sys", sysInit,
       "i|mqtt", mqttInit,
       "i|ble", bleInit,
       "i|gps", gpsInit,
       "i|can", canInit);
  json(sysJson, "firmware", FIRMWARE_VERSION, "i|configFreeMem", Config.getConfigFreeMem(), "o|init", initJson);
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
