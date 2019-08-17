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
  delay(5000);  // to see beginning of the login
#endif
  Watchdog.enable(WATCHDOG_TIMEOUT);

  Pins.begin();
  if (!ECCX08.begin()) {
    log("ERROR", "No ECCX08 present");
  }
  if (!SD.begin(SD_CS_PIN)) {
    log("ERROR", "Failed to initialize SD Library");
  }
  Logger.begin();
  Config.load();
  Command.begin();
  System.begin();
  Mqtt.begin();
  Mqtt.poll();
  Bluetooth.begin();
  Gps.begin();
  Can.begin();
  System.sendInfo();
}

void loop() {
  Watchdog.enable(WATCHDOG_TIMEOUT);
  if (!System.stayAwake()) {
    System.sleep(1);
  }
  System.keepTime();
  // Mqtt.poll();
  System.poll();
  Bluetooth.poll();
  Can.poll();
}
