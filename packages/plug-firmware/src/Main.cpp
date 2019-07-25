#include <Adafruit_SleepyDog.h>
#include <Arduino.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Config.h"
#include "Gps.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

void setup() {
#ifndef DEBUG
  Watchdog.enable(16 * 1000);
#endif
  Pins.setup();
  Logger.setup();
  Config.load();
  Mqtt.setup();
  System.setup();
  Can.setup();
  Bluetooth.setup();
  Gps.setup();
}

void loop() {
  Watchdog.reset();
  Mqtt.poll();
  Bluetooth.poll();
  Gps.poll();
  System.poll();
  System.kickWatchdogAndSleep();
}
