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
  Watchdog.enable(WATCHDOG_TIMEOUT);
  Pins.setup();
  Logger.setup();
  Config.load();
#ifdef ARDUINO_SAMD_MKR1000
  Mqtt.setup();
  Mqtt.poll();
#endif
  System.setup();
  Bluetooth.setup();
  Can.setup();
  Gps.setup();
  Gps.poll();
}

void loop() {
  System.sleep();
#ifdef ARDUINO_SAMD_MKR1000
  Mqtt.poll();
#endif
  System.poll();
  Bluetooth.poll();
  Can.poll();
}
