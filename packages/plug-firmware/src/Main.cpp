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
  // Mqtt.poll();
  System.sleep();  // shouuld be after Gps.poll() and Mqtt.poll() to have time available
  System.poll();   // should be after System.sleep() to have time available
  Bluetooth.poll();
#ifdef ARDUINO_SAMD_WAIVE1000
  Can.poll();
  Gps.poll();
#endif
}
