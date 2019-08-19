#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>

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
#if DEBUG
  delay(3000);  // to see beginning of the login
#endif
  Watchdog.enable(WATCHDOG_TIMEOUT);

  Pins.begin();
  if (!ECCX08.begin()) {
    logError("No ECCX08 present");
  }
  if (!SD.begin(SD_CS_PIN)) {
    logError("Failed to initialize SD Library");
  }
  Config.load();
  Logger.begin();
  Command.begin();
  System.begin();
  Mqtt.begin();
#ifdef ARDUINO_SAMD_MKR1000
  Mqtt.poll();
#else
  Internet.connect();
#endif
  Bluetooth.begin();
  Gps.begin();
  Can.begin();
  System.sendInfo();
}

void loop() {
  Watchdog.reset();
  if (!System.stayAwake()) {
    System.sleep(1);
  }
  System.keepTime();
#ifdef ARDUINO_SAMD_MKR1000
  Mqtt.poll();
#endif
  System.poll();
  Bluetooth.poll();
  Can.poll();
}
