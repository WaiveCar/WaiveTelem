#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Config.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

void setup() {
  Watchdog.enable(WATCHDOG_TIMEOUT);
  Pins.setup();
  Logger.setup();
  Config.load();
#ifdef ARDUINO_SAMD_MKR1000
  Mqtt.setup();
  Mqtt.poll();
#else
  Internet.connect();
  while (!ECCX08.begin()) {
    logError(F("No ECCX08 present"));
    delay(5000);
  }
#endif
  System.setTime(Internet.getTime());
  Gps.setup();
  Bluetooth.setup();
  Can.setup();
  System.setup();
  System.sendInfo();
}

void loop() {
  Watchdog.enable(WATCHDOG_TIMEOUT);
  if (!System.getStayAwake()) {
    System.sleep(1);
  }
#ifdef ARDUINO_SAMD_MKR1000
  Mqtt.poll();
#endif
  System.poll();
  Bluetooth.poll();
  Can.poll();
}
