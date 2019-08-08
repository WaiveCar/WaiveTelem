#include <Adafruit_SleepyDog.h>
#include <Arduino.h>

#include <ACAN2515.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Config.h"
#include "Gps.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

void setup() {
  // Watchdog.enable(WATCHDOG_TIMEOUT);
  Pins.setup();
  Logger.setup();
  Config.load();
  // Mqtt.setup();
  System.setup();
  Bluetooth.setup();
#ifdef ARDUINO_SAMD_WAIVE1000
  Can.setup();
  Gps.setup();
#endif
}

void loop() {
  System.sleep();
  // Mqtt.poll();
  System.poll();
  Bluetooth.poll();
#ifdef ARDUINO_SAMD_WAIVE1000
  Can.poll();
  Gps.poll();
#endif
}
